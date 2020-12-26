#include "../header/mechan.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_lowercase.h"

namespace mechan
{
	class SynonymParser
	{
	private:
		Mechan *_mechan						= nullptr;
		ir::S2STDatabase *_word2exsyngroup	= nullptr;
		FILE *_synonym						= nullptr;
		std::vector<unsigned int> _buffer;
		std::vector<unsigned int> _groups;
		std::vector<std::string> _words;

		static bool _merge(const unsigned int *source, size_t source_size, std::vector<unsigned int> *dest)	noexcept;
		void _extract_groups(const std::string lowercase_word)												noexcept;
		void _write_groups()																				noexcept;

	public:
		SynonymParser(Mechan *mechan)																		noexcept;
		bool parse()																						noexcept;
		~SynonymParser()																					noexcept;
	};
}

bool mechan::SynonymParser::_merge(const unsigned int *source, size_t source_size, std::vector<unsigned int> *dest) noexcept
{
	bool adding = false;
	for (size_t i = 0; i < source_size; i++)
	{
		bool match = false;
		size_t place_in_dest = dest->size();
		for (size_t j = 0; j < dest->size(); j++)
		{
			if (source[i] < dest->at(j))
			{
				place_in_dest = j;
				break;
			}
			else if (source[i] == dest->at(j))
			{
				match = true;
				break;
			}
		}
		if (!match)
		{
			dest->insert(dest->begin() + place_in_dest, source[i]);
			adding = true;
		}
	}
	return adding;
}

void mechan::SynonymParser::_extract_groups(const std::string lowercase_word) noexcept
{
	_mechan->morphology()->word_info(lowercase_word, &_buffer);
	_merge(_buffer.data(), (unsigned int)_buffer.size(), &_groups);
}

void mechan::SynonymParser::_write_groups() noexcept
{
	ir::ConstBlock groups(_groups.data(), _groups.size() * sizeof(unsigned int));

	for (unsigned int i = 0; i < _words.size(); i++)
	{
		ir::ConstBlock keyword(_words[i].data(), _words[i].size());
		if (_word2exsyngroup->insert(keyword, groups, ir::Database::insert_mode::not_existing) == ir::ec::key_already_exists)
		{
			ir::ConstBlock old_groups;
			_word2exsyngroup->read(keyword, &old_groups);
			_buffer = _groups;
			if (_merge((unsigned int*)old_groups.data(), (unsigned int)old_groups.size() / sizeof(unsigned int), &_buffer))
			{
				ir::ConstBlock merged_groups(_buffer.data(), _buffer.size() * sizeof(unsigned int));
				_word2exsyngroup->insert(keyword, merged_groups);
			}
		}
	}
}

mechan::SynonymParser::SynonymParser(Mechan *mechan) noexcept : _mechan(mechan)
{}

bool mechan::SynonymParser::parse() noexcept
{
	_word2exsyngroup = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2exsyngroup", ir::Database::create_mode::neww, nullptr);
	if (!_word2exsyngroup->ok()) return false;
	_word2exsyngroup->set_ram_mode(true, true);

	_synonym = _wfopen(WIDE_MECHAN_DIR "\\data\\synonym.txt", L"r");
	if (_synonym == nullptr) return false;

	_mechan->print_event_log("Morphology file found");
	fseek(_synonym, 0, SEEK_END);
	unsigned int file_size = ftell(_synonym);
	fseek(_synonym, 0, SEEK_SET);
	unsigned int reported_procent = 0;

	_words.push_back(std::string());
	bool request_new = true;
	bool request_space = false;
	while (true)
	{
		unsigned int procent = (unsigned int)(100.0 * ftell(_synonym) / file_size);
		if (procent > reported_procent)
		{
			reported_procent = procent;
			char report[64];
			sprintf(report, "%u%% of synonym file processed", reported_procent);
			_mechan->print_event_log(report);
		}

		char c;
		if (fread(&c, 1, 1, _synonym) == 0) break;
		else if (c == '\r') {}
		else if (c == '\n')
		{
			if (!_words.empty()) _extract_groups(_words.back());
			if (!_groups.empty()) _write_groups();
			_groups.resize(0);
			_words.resize(0);
			request_new = true;
		}
		else if (c == '|')
		{
			if (!_words.empty()) _extract_groups(_words.back());
			request_new = true;
		}
		else if (!is_lowercase_alphanumeric(lowercase(c)))
		{
			request_space = true;
		}
		else
		{
			if (request_new) { _words.push_back(std::string()); request_new = false; request_space = false; }
			if (request_space) { _words.back().push_back(' '); request_space = false; }
			_words.back().push_back(lowercase(c));
		}
	}
	if (!_words.empty()) _extract_groups(_words.back());
	if (!_groups.empty()) _write_groups();
	_mechan->print_event_log("Optimizing synonym database");
	_word2exsyngroup->optimize();
	return true;
}

mechan::SynonymParser::~SynonymParser() noexcept
{
	if (_synonym != nullptr) fclose(_synonym);
	if (_word2exsyngroup != nullptr) delete _word2exsyngroup;
}

mechan::Synonym::Synonym(Mechan *mechan) noexcept  : _mechan(mechan)
{
	ir::ec code;
	_word2exsyngroup = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2exsyngroup", ir::Database::create_mode::read, &code);
	if (code == ir::ec::ok)
	{
		_mechan->print_event_log("Synonym database found");
		_word2exsyngroup->set_ram_mode(true, true);
		return;
	}

	delete _word2exsyngroup;
	bool parsed;
	{
		SynonymParser parser(_mechan);
		parsed = parser.parse();
	}
	if (parsed)
	{
		_word2exsyngroup = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\word2exsyngroup", ir::Database::create_mode::read, &code);
		if (_word2exsyngroup->ok())
		{
			_mechan->print_event_log("Morphology database found");
			_word2exsyngroup->set_ram_mode(true, true);
		}
	}
	else _word2exsyngroup = nullptr;
}

bool mechan::Synonym::ok() const noexcept
{
	return _word2exsyngroup != nullptr && _word2exsyngroup->ok();
}

void mechan::Synonym::extended_syngroup(const std::string lowercase_word , std::vector<unsigned int> *groups) const noexcept
{
	ir::ConstBlock keyword(lowercase_word.c_str(), lowercase_word.size());
	ir::ConstBlock groups_data;
	if (_word2exsyngroup->read(keyword, &groups_data) == ir::ec::ok)
	{
		groups->resize(groups_data.size() / sizeof(unsigned int));
		memcpy(&groups->at(0), groups_data.data(), groups_data.size());
	}
	else groups->resize(0);
}

mechan::Synonym::~Synonym() noexcept
{
	if (_word2exsyngroup != nullptr) delete _word2exsyngroup;
}