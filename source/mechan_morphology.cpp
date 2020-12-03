#include "../header/mechan_morphology.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_lowercase.h"
#include <ir_resource/ir_file_resource.h>
#include <assert.h>

void mechan::Morphology::Parser::_skip_space()
{
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _morphology) == 0) return;
		else if (c == ' ') {}
		else
		{
			fseek(_morphology, SEEK_CUR, -1);
			return;
		}
	}
}

void mechan::Morphology::Parser::_skip_space_delimiter()
{
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _morphology) == 0) return;
		else if (c == ' ' || c == '|') {}
		else
		{
			fseek(_morphology, SEEK_CUR, -1);
			return;
		}
	}
}

void mechan::Morphology::Parser::_skip_space_asterisk()
{
	_deprecated = false;
	while (true)
	{
		char c;
		assert(fread(&c, 1, 1, _morphology) != 0);
		if (c == ' ') {}
		else if (c == '*') _deprecated = true;
		else
		{
			fseek(_morphology, SEEK_CUR, -1);
			return;
		}
	}
}

bool mechan::Morphology::Parser::_skip_line()
{
	unsigned int lncount = 0;
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _morphology) == 0) return true;
		else if (c == ' ' || c == '|' || c == '\r') {}
		else if (c == '\n') lncount++;
		else
		{
			if (lncount != 0)
			{
				fseek(_morphology, SEEK_CUR, -1);
				return lncount > 1;
			}
		}
	}
}

void mechan::Morphology::Parser::_read_word(bool spelling)
{
	//Begin or continue word recording
	if (spelling) _item_buffer.back().spelling = (unsigned int)_string_buffer.size();
	else
	{
		OffsetGroupItem item;
		item.lowercase_word = (unsigned int)_string_buffer.size();
		_item_buffer.push_back(item);
	}

	//Record
	while (true)
	{
		char c;
		assert(fread(&c, 1, 1, _morphology) != 0);
		if (c == ' ' || c == '|') break;
		else _string_buffer.push_back(c);
	}

	//Cut delimiters off
	while (_string_buffer.back() == ' ' || _string_buffer.back() == '|') _string_buffer.pop_back();

	//End
	_string_buffer.push_back('\0');

	//Lowercase && add to word2group
	if (!spelling)
	{
		const char *lowercase_word = _string_buffer.data() + _item_buffer.back().lowercase_word;
		lowercase(_string_buffer.data() + _item_buffer.back().lowercase_word);
		ir::ConstBlock key((unsigned int)_string_buffer.size() - _item_buffer.back().lowercase_word - 1, lowercase_word);
		ir::ConstBlock value(sizeof(unsigned int), &_group);
		ir::ec code = _word2group->insert(key, value, ir::S2STDatabase::insert_mode::not_existing);
		if (code == ir::ec::key_already_exists)
		{
			ir::ConstBlock old_groups;
			_word2group->read(key, &old_groups);

			bool match = false;
			for (unsigned int i = 0; i < old_groups.size / sizeof(unsigned int); i++)
			{
				if (*((unsigned int *)old_groups.data + i) == _group)
				{
					match = true;
					break;
				}
			}

			if (!match)
			{
				_continuous_buffer.resize(old_groups.size + sizeof(unsigned int));
				memcpy(&_continuous_buffer[0], old_groups.data, old_groups.size);
				memcpy(&_continuous_buffer[old_groups.size], &_group, sizeof(unsigned int));
				ir::ConstBlock new_groups((unsigned int)_continuous_buffer.size(), _continuous_buffer.data());
				_word2group->insert(key, new_groups);
			}
		}
	}
}

void mechan::Morphology::Parser::_add_characteristic()
{
	//Cut spaces off
	while (!_characteristics.empty() && _characteristics_buffer.back() == ' ') _characteristics_buffer.pop_back();
	if (_characteristics.empty()) return;

	//Adding word to possible descriptions
	bool found = false;
	unsigned char index;
	for (unsigned char i = 0; i < _characteristics.size(); i++)
	{
		if (_characteristics[i] == _characteristics_buffer)
		{
			index = i;
			found = true;
			break;
		}
	}
	if (!found)
	{
		_characteristics.push_back(_characteristics_buffer);
		index = (unsigned char)(_characteristics.size() - 1);
	}

	//Adding characteristic to description
	_string_buffer.push_back((char)index);

	//Clearing buffer
	_characteristics_buffer.clear();
}

void mechan::Morphology::Parser::_read_characteristics()
{
	//Begin characteristics record
	_item_buffer.back().characteristics = (unsigned int)_string_buffer.size();

	//Record
	while (true)
	{
		char c;
		assert(fread(&c, 1, 1, _morphology) != 0);
		if (c == ' ')
		{
			_add_characteristic();
			_skip_space();
		}
		else if (c == '|')
		{
			_add_characteristic();
			_skip_space_delimiter();
			break;
		}
		else _characteristics_buffer.push_back(c);
	}

	//End
	_string_buffer.push_back('\0');
}

bool mechan::Morphology::Parser::parse()
{
	ir::ec code;
	_word2group = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2group", ir::Database::create_mode::neww, &code);
	if (code != ir::ec::ok) return false;

	_group2data = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\group2data", ir::Database::create_mode::neww, &code);
	if (code != ir::ec::ok) return false;

	_morphology = _wfopen(WIDE_MECHAN_DIR "\\data\\morphology.txt", L"r"); 
	if (_morphology == nullptr) return false;

	while (feof(_morphology))
	{
		_skip_space_asterisk();
		_read_word(false);
		_read_characteristics();
		_read_word(true);
		if (_skip_line())
		{
			_continuous_buffer.resize(sizeof(unsigned int) + sizeof(OffsetGroupItem) + _string_buffer.size());
			ir::ConstBlock group_data((unsigned int)_continuous_buffer.size(), _continuous_buffer.data());
			if (_group2data->insert(_group, group_data) != ir::ec::ok) return true;
			_group++;
		}
	}

	return true;
}

mechan::Morphology::Parser::~Parser()
{
	if (_word2group != nullptr) delete _word2group;
	if (_group2data != nullptr) delete _group2data;
	if (_morphology != nullptr) fclose(_morphology);
}

mechan::Morphology::Morphology()
{
	//Open already existing database
	ir::ec word2group_code;
	_word2group = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2group", ir::Database::create_mode::read, &word2group_code);
	ir::ec group2data_code;
	_group2data = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\group2data", ir::Database::create_mode::read, &group2data_code);
	if (word2group_code == ir::ec::ok && group2data_code == ir::ec::ok)
	{
		_word2group->set_ram_mode(true, true);
		_group2data->set_ram_mode(true, true);
		_ok = true;
		return;
	}

	//Create new database && open it again
	Parser parser;
	if (parser.parse())
	{
		_word2group = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2group", ir::Database::create_mode::read, &word2group_code);
		_group2data = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\group2data", ir::Database::create_mode::read, &group2data_code);
		if (word2group_code == ir::ec::ok && group2data_code == ir::ec::ok)
		{
			_word2group->set_ram_mode(true, true);
			_group2data->set_ram_mode(true, true);
			_ok = true;
		}
	}
}

void mechan::Morphology::word_info(const char *lowercase_word, std::vector<unsigned int> *groups) const noexcept
{
	ir::ConstBlock key((unsigned int)strlen(lowercase_word), lowercase_word);
	ir::ConstBlock data;
	if (_word2group->read(key, &data) != ir::ec::ok) groups->resize(0);
	else
	{
		groups->resize(data.size / sizeof(unsigned int));
		memcpy(&groups[0], data.data, data.size);
	}
	
}

void mechan::Morphology::group_info(unsigned int group, std::vector<GroupItem> *items) const noexcept
{
	ir::ConstBlock data;
	assert(_group2data->read(group, &data) == ir::ec::ok);
	unsigned int items_number;
	memcpy(&items_number, data.data, sizeof(unsigned int));
	items->resize(items_number);
	for (unsigned int i = 0; i < items_number; i++)
	{
		OffsetGroupItem item;
		memcpy(&item, (char*)data.data + sizeof(unsigned int) + i * sizeof(OffsetGroupItem), sizeof(OffsetGroupItem));
		const char *string_begin = (char*)data.data + sizeof(unsigned int) + items_number * sizeof(OffsetGroupItem);
		items->at(i).lowercase_word = string_begin + item.lowercase_word;
		items->at(i).characteristics = (Characteristic*)string_begin + item.characteristics;
		items->at(i).spelling = string_begin + item.spelling;
	}
}

mechan::Morphology::~Morphology()
{
	if (_word2group != nullptr) delete _word2group;
	if (_group2data != nullptr) delete _group2data;
}