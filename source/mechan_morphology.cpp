#include "../header/mechan.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_lowercase.h"
#include <assert.h>

namespace mechan
{
	class MorphologyParser
	{
	private:
		Mechan *_mechan					= nullptr;
		ir::S2STDatabase *_word2group	= nullptr;
		ir::N2STDatabase *_group2data	= nullptr;
		FILE *_morphology				= nullptr;
		unsigned int _group				= 0;
		bool _deprecated				= false;
		std::vector<std::string> _characteristics;
		std::string _characteristic;
		std::vector<Morphology::OffsetGroupItem> _items;
		std::vector<char> _buffer;
		std::vector<char> _items_data;

		void _skip_space()												noexcept;
		void _skip_space_delimiter()									noexcept;
		void _skip_space_asterisk()										noexcept;
		bool _merge(ir::ConstBlock old_groups, unsigned int new_group)	noexcept;
		void _read_word(bool spelling)									noexcept;
		void _read_characteristics()									noexcept;
		void _add_characteristic()										noexcept;
		bool _skip_line()												noexcept;

	public:
		MorphologyParser(Mechan *mechan)								noexcept;
		bool parse()													noexcept;
		~MorphologyParser()												noexcept;
	};
}

void mechan::MorphologyParser::_skip_space() noexcept
{
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _morphology) == 0) return;
		else if (c == ' ') {}
		else
		{
			fseek(_morphology, -1, SEEK_CUR);
			return;
		}
	}
}

void mechan::MorphologyParser::_skip_space_delimiter() noexcept
{
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _morphology) == 0) return;
		else if (c == ' ' || c == '|') {}
		else
		{
			fseek(_morphology, -1, SEEK_CUR);
			return;
		}
	}
}

void mechan::MorphologyParser::_skip_space_asterisk() noexcept
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
			fseek(_morphology, -1, SEEK_CUR);
			return;
		}
	}
}

bool mechan::MorphologyParser::_skip_line() noexcept
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
				fseek(_morphology, -1, SEEK_CUR);
				return lncount > 1;
			}
		}
	}
}

bool mechan::MorphologyParser::_merge(ir::ConstBlock old_groups, unsigned int new_group) noexcept
{
	bool match = false;
	for (size_t i = 0; i < old_groups.size() / sizeof(unsigned int); i++)
	{
		if (*((unsigned int *)old_groups.data() + i) == _group)
		{
			match = true;
			break;
		}
	}

	if (!match)
	{
		_buffer.resize(old_groups.size() + sizeof(unsigned int));
		memcpy(&_buffer[0], old_groups.data(), old_groups.size());
		memcpy(&_buffer[old_groups.size()], &_group, sizeof(unsigned int));
		return true;
	}
	else return false;
}

void mechan::MorphologyParser::_read_word(bool spelling) noexcept
{
	//Begin or continue word recording
	if (spelling) _items.back().spelling = (unsigned int)_items_data.size();
	else
	{
		Morphology::OffsetGroupItem item;
		item.lowercase_word = (unsigned int)_items_data.size();
		_items.push_back(item);
	}

	//Record
	while (true)
	{
		char c;
		assert(fread(&c, 1, 1, _morphology) != 0);
		if (c == '|') break;
		else if (spelling) _items_data.push_back(c);
		else _items_data.push_back(lowercase(c));
	}

	//Cut delimiters off
	while (_items_data.back() < 0xE0) _items_data.pop_back();

	//End
	_items_data.push_back('\0');

	//Lowercase && add to word2group
	if (!spelling)
	{
		const char *lowercase_word = _items_data.data() + _items.back().lowercase_word;
		ir::ConstBlock keyword(lowercase_word, _items_data.size() - _items.back().lowercase_word - 1);
		ir::ConstBlock new_group(&_group, sizeof(unsigned int));
		ir::ec code = _word2group->insert(keyword, new_group, ir::S2STDatabase::insert_mode::not_existing);
		if (code == ir::ec::key_already_exists)
		{
			ir::ConstBlock old_groups;
			_word2group->read(keyword, &old_groups);
			if (_merge(old_groups, _group))
			{
				ir::ConstBlock merged_groups(_buffer.data(), _buffer.size());
				_word2group->insert(keyword, merged_groups);
			}
		}
	}
}

void mechan::MorphologyParser::_add_characteristic() noexcept
{
	//Cut spaces off
	while (!_characteristic.empty() && _characteristic.back() < 0xE0) _characteristic.pop_back();
	if (_characteristic.empty()) return;

	//Adding word to possible descriptions
	bool found = false;
	unsigned char index;
	for (unsigned char i = 0; i < _characteristics.size(); i++)
	{
		if (_characteristics[i] == _characteristic)
		{
			index = i;
			found = true;
			break;
		}
	}
	if (!found)
	{
		_characteristics.push_back(_characteristic);
		index = (unsigned char)(_characteristics.size() - 1);
	}

	//Adding characteristic to description
	_items_data.push_back((char)index);

	//Clearing buffer
	_characteristic.resize(0);
}

void mechan::MorphologyParser::_read_characteristics() noexcept
{
	//Begin characteristics record
	_items.back().characteristics = (unsigned int)_items_data.size();

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
		else _characteristic.push_back(c);
	}

	//End
	_items_data.push_back('\0');
}

mechan::MorphologyParser::MorphologyParser(Mechan *mechan) noexcept : _mechan(mechan)
{}

bool mechan::MorphologyParser::parse() noexcept
{
	_word2group = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2group", ir::Database::create_mode::neww, nullptr);
	if (!_word2group->ok()) return false;
	_word2group->set_ram_mode(true, true);

	_group2data = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\group2data", ir::Database::create_mode::neww, nullptr);
	if (!_group2data->ok()) return false;
	_group2data->set_ram_mode(true, true);

	_morphology = _wfopen(WIDE_MECHAN_DIR "\\data\\morphology.txt", L"r"); 
	if (_morphology == nullptr) return false;

	_mechan->print_event_log("Morphology file found");
	fseek(_morphology, 0, SEEK_END);
	unsigned int file_size = ftell(_morphology);
	fseek(_morphology, 0, SEEK_SET);
	unsigned int reported_procent = 0;

	while (!feof(_morphology))
	{
		unsigned int procent = (unsigned int)(100.0 * ftell(_morphology) / file_size);
		if (procent > reported_procent)
		{
			reported_procent = procent;
			char report[64];
			sprintf(report, "%u%% of morphology file processed", reported_procent);
			_mechan->print_event_log(report);
		}

		_skip_space_asterisk();
		_read_word(false);
		_skip_space_delimiter();
		_read_characteristics();
		_skip_space_delimiter();
		_read_word(true);
		if (_skip_line())
		{
			unsigned int item_number = (unsigned int)_items.size();
			_buffer.resize(
				sizeof(unsigned int) +
				item_number * sizeof(Morphology::OffsetGroupItem) +
				_items_data.size());
			memcpy(&_buffer[0],
				&item_number,
				sizeof(unsigned int));
			memcpy(&_buffer[sizeof(unsigned int)],
				_items.data(),
				item_number * sizeof(Morphology::OffsetGroupItem));
			memcpy(&_buffer[sizeof(unsigned int) + item_number * sizeof(Morphology::OffsetGroupItem)],
				_items_data.data(),
				_items_data.size());
			ir::ConstBlock group_data(_buffer.data(), _buffer.size());
			if (_group2data->insert(_group, group_data) != ir::ec::ok) return true;
			_items.resize(0);
			_items_data.resize(0);
			_group++;
		}
	}

	return true;
}

mechan::MorphologyParser::~MorphologyParser()
{
	if (_word2group != nullptr) delete _word2group;
	if (_group2data != nullptr) delete _group2data;
	if (_morphology != nullptr) fclose(_morphology);
}

mechan::Morphology::Morphology(Mechan *mechan) noexcept : _mechan(mechan)
{
	//First try
	_word2group = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2group", ir::Database::create_mode::read, nullptr);
	_group2data = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\group2data", ir::Database::create_mode::read, nullptr);
	if (_word2group->ok() && _group2data->ok())
	{
		_mechan->print_event_log("Morphology database found");
		_word2group->set_ram_mode(true, true);
		_group2data->set_ram_mode(true, true);
		return;
	}

	//Second try
	delete _word2group;
	delete _group2data;
	bool parsed;
	{
		MorphologyParser parser(_mechan);
		parsed = parser.parse();
	}
	
	if (parsed)
	{
		_word2group = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2group", ir::Database::create_mode::read, nullptr);
		_group2data = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\group2data", ir::Database::create_mode::read, nullptr);
		if (_word2group->ok() && _group2data->ok())
		{
			_mechan->print_event_log("Morphology database found");
			_word2group->set_ram_mode(true, true);
			_group2data->set_ram_mode(true, true);
		}
	}
	else
	{
		_word2group = nullptr;
		_group2data = nullptr;
	}
}

bool mechan::Morphology::ok() const noexcept
{
	return (_word2group != nullptr && _word2group->ok() && _group2data != nullptr && _group2data->ok());
}

void mechan::Morphology::word_info(const std::string lowercase_word, std::vector<unsigned int> *groups) const noexcept
{
	ir::ConstBlock key(lowercase_word.c_str(), lowercase_word.size());
	ir::ConstBlock data;
	if (_word2group->read(key, &data) != ir::ec::ok) groups->resize(0);
	else
	{
		groups->resize(data.size() / sizeof(unsigned int));
		memcpy(&groups->at(0), data.data(), data.size());
	}
}

void mechan::Morphology::group_info(unsigned int group, std::vector<GroupItem> *items) const noexcept
{
	ir::ConstBlock data;
	assert(_group2data->read(group, &data) == ir::ec::ok);
	unsigned int items_number;
	memcpy(&items_number, data.data(), sizeof(unsigned int));
	items->resize(items_number);
	for (unsigned int i = 0; i < items_number; i++)
	{
		OffsetGroupItem item;
		memcpy(&item, (char*)data.data() + sizeof(unsigned int) + i * sizeof(OffsetGroupItem), sizeof(OffsetGroupItem));
		const char *string_begin = (char*)data.data() + sizeof(unsigned int) + items_number * sizeof(OffsetGroupItem);
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