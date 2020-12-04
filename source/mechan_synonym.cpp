#include "../header/mechan_synonym.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_lowercase.h"

namespace mechan
{
	class Parser
	{
	private:
		ir::S2STDatabase *_word2exsyngroup = nullptr;
		FILE *_synonym = nullptr;
		std::vector<char> _continuous_buffer;

		void _add_group(const std::string word, unsigned int group) noexcept;

	public:
		bool parse() noexcept;
		~Parser() noexcept;
	};
}

void mechan::Parser::_add_group(const std::string word, unsigned int group) noexcept
{
	std::string lower = lowercase(word);
	while (!lower.empty() && lower.back() == ' ') lower.pop_back();
	if (lower.empty()) return;
	ir::ConstBlock key((unsigned int)lower.size() - 1, lower.c_str());
	ir::ConstBlock data(sizeof(unsigned int), &group);
	if (_word2exsyngroup->insert(key, data, ir::Database::insert_mode::not_existing) == ir::ec::key_already_exists)
	{
		ir::ConstBlock old_groups;
		_word2exsyngroup->read(key, &old_groups);
		bool match = false;
		for (unsigned int i = 0; i < old_groups.size / sizeof(unsigned int); i++)
		{
			if (*((unsigned int *)old_groups.data + i) == group)
			{
				match = true;
				break;
			}
		}

		if (!match)
		{
			_continuous_buffer.resize(old_groups.size + sizeof(unsigned int));
			memcpy(&_continuous_buffer[0], old_groups.data, old_groups.size);
			memcpy(&_continuous_buffer[old_groups.size], &group, sizeof(unsigned int));
			ir::ConstBlock new_groups((unsigned int)_continuous_buffer.size(), _continuous_buffer.data());
			_word2exsyngroup->insert(key, new_groups);
		}
	}
}

bool mechan::Parser::parse() noexcept
{
	_synonym = fopen(MECHAN_DIR, "r");
	if (_synonym == nullptr) return false;
	_word2exsyngroup = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\extended_synonym", ir::Database::create_mode::neww, nullptr);
	if (!_word2exsyngroup->ok()) return false;
	unsigned int group = 0;
	std::string buffer;
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _synonym) == 0) break;
		else if (c == '\r') {}
		else if (c == '\n')
		{
			_add_group(buffer, group);
			buffer.resize(0);
			group++;
		}
		else if (c == '|')
		{
			_add_group(buffer, group);
			buffer.resize(0);
		}
		else if (c == ' ')
		{
			if (!buffer.empty()) buffer.push_back(c);
		}
		else buffer.push_back(c);
	}
	while (buffer.back() == ' ') buffer.pop_back();
	_add_group(lowercase(buffer), group);
	return true;
}

mechan::Parser::~Parser() noexcept
{
	if (_synonym != nullptr) fclose(_synonym);
	if (_word2exsyngroup != nullptr) delete _word2exsyngroup;
}

mechan::Synonym::Synonym() noexcept
{
	ir::ec code;
	_word2exsyngroup = new ir::S2STDatabase(WIDE_MECHAN_DIR, ir::Database::create_mode::read, &code);
	if (code == ir::ec::ok) return;

	delete _word2exsyngroup;
	bool parsed;
	{
		Parser parser;
		parsed = parser.parse();
	}
	if (parsed) _word2exsyngroup = new ir::S2STDatabase(WIDE_MECHAN_DIR, ir::Database::create_mode::read, &code);
	else _word2exsyngroup = nullptr;
}

bool mechan::Synonym::ok() const noexcept
{
	return _word2exsyngroup != nullptr && _word2exsyngroup->ok();
}

void mechan::Synonym::extended_syngroup(const std::string lowercase_word , std::vector<unsigned int> *groups) const noexcept
{
	ir::ConstBlock key((unsigned int)lowercase_word.size() - 1, lowercase_word.c_str());
	ir::ConstBlock data;
	if (_word2exsyngroup->read(key, &data) == ir::ec::ok)
	{
		groups->resize(data.size / sizeof(unsigned int));
		memcpy(&groups->at(0), data.data, data.size);
	}
	else groups->resize(0);
}

mechan::Synonym::~Synonym() noexcept
{
	if (_word2exsyngroup != nullptr) delete _word2exsyngroup;
}