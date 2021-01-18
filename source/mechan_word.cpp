#include "../header/mechan.h"
#include "../header/mechan_character.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_parse.h"
#include <assert.h>
#include <ir_resource/ir_file_resource.h>

bool mechan::Word::MorphologyCharacteristics::get(MorphologyCharacteristic c) const noexcept
{
	unsigned int ic = (unsigned int)c;
	if (ic >= 64) return _c & (1 << (ic - 64));
	else if (ic >= 32) return _b & (1 << (ic - 32));
	else return _a & (1 << ic);
}

void mechan::Word::MorphologyCharacteristics::set(MorphologyCharacteristic c, bool v) noexcept
{
	unsigned int ic = (unsigned int)c;
	if (v)
	{
		if (ic >= 64) _c |= (1 << (ic - 64));
		else if (ic >= 32) _b |= (1 << (ic - 32));
		else _a |= (1 << ic);
	}
	else
	{
		if (ic >= 64) _c &= ~(1 << (ic - 64));
		else if (ic >= 32) _b &= ~(1 << (ic - 32));
		else _a &= ~(1 << ic);
	}
}

unsigned int *mechan::Word::WordInfo::morphology_groups() noexcept
{
	return (unsigned int*)(((char*)this) + sizeof(WordInfo));
}

const unsigned int *mechan::Word::WordInfo::morphology_groups() const noexcept
{
	return (unsigned int*)(((char*)this) + sizeof(WordInfo));
}

unsigned int mechan::Word::WordInfo::synonym_group_number(unsigned int size) const noexcept
{
	return (size - sizeof(WordInfo) - morphology_group_number * sizeof(unsigned int)) / sizeof(unsigned int);
}

unsigned int *mechan::Word::WordInfo::synonym_groups() noexcept
{
	return (unsigned int*)(((char*)this) + sizeof(WordInfo) + morphology_group_number * sizeof(unsigned int));
}

const unsigned int *mechan::Word::WordInfo::synonym_groups() const noexcept
{
	return (unsigned int*)(((char*)this) + sizeof(WordInfo) + morphology_group_number * sizeof(unsigned int));
}

bool mechan::Word::_load() noexcept
{
	_words = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\words", ir::Database::create_mode::read, nullptr);
	if (!_words->ok() || _words->set_ram_mode(true, true) != ir::ec::ok)
	{
		delete _words;
		_words = nullptr;
		return false;
	}

	return true;
}

bool mechan::Word::_parse_dialog() noexcept
{
	std::string message;
	Parsed parsed;
	unsigned int reported = 0;
	
	for (unsigned int index = 0; index < _mechan->dialog()->count(); index++)
	{
		//Report success
		if ((unsigned int)(100.0 * index / _mechan->dialog()->count()) > reported)
		{
			reported = (unsigned int)(100.0 * index / _mechan->dialog()->count());
			char buffer[64];
			sprintf(buffer, "Dialog words counting %u", reported);
			_mechan->print_event_log(buffer);
		}

		//Parse message
		if (!_mechan->dialog()->message(index, &message)) continue;
		parse_punctuation(message, &parsed);
		
		//Encount each word in message
		for (unsigned int j = 0; j < parsed.words.size(); j++)
		{
			if (parsed.words[j].uppercase)
				_unpacked_words[parsed.words[j].lowercase].uppercase_occurence_number++;
			else
				_unpacked_words[parsed.words[j].lowercase].lowercase_occurence_number++;
		}
	}
	return true;
}

void mechan::Word::_parse_morphology_add(unsigned int group, const std::string lowercase_word, MorphologyCharacteristics ch) noexcept
{
	if (lowercase_word.empty() || _unpacked_words.count(lowercase_word) == 0) return;

	_unpacked_morphology_groups.resize(group + 1);
	_unpacked_morphology_groups[group].lowercase_words.insert(lowercase_word);
	if (_unpacked_words[lowercase_word].morphology_characteristics.count(group) == 0)
		_unpacked_words[lowercase_word].morphology_characteristics[group] = ch;
}

bool mechan::Word::_parse_morphology() noexcept
{
	enum class State
	{
		double_endline_wait_word,
		endline_wait_word,
		word_wait_delimiter,
		word_space_wait_delimiter,
		space_wait_characteristic,
		characteristic_wait_space_delimiter,
		wait_endline
	};
	
	//Open file
	ir::FileResource morphology = fopen(MECHAN_DIR "\\data\\morphology.txt", "r");
	_mechan->print_event_log("Morphology file found");
	if (morphology == nullptr) return false;
	fseek(morphology, 0, SEEK_END);
	unsigned int morphology_size = ftell(morphology);
	fseek(morphology, 0, SEEK_SET);
	
	//Init variables
	State state = State::double_endline_wait_word;
	std::string lowercase_word;
	std::string text_characteristic;
	std::vector<std::string> text_characteristics;
	MorphologyCharacteristics characteristics;
	unsigned int group = 0;
	unsigned int reported = 0;
	unsigned int position = 0;

	//Parse
	while (true)
	{
		//Read and process symbol
		char c;
		if (fread(&c, 1, 1, morphology) == 0)
		{
			_parse_morphology_add(group, lowercase_word, characteristics);
			break;
		}
		else switch (state)
		{
		case State::double_endline_wait_word:	
		{
			if (is_alphanumeric(c))
			{
				lowercase_word.push_back(lowercase(c));
				state = State::word_wait_delimiter;
			}
			else if (c == '*')
			{
				characteristics.set(MorphologyCharacteristic::deprecated, true);
			}
			break;
		}
		
		case State::endline_wait_word:
		{
			if (is_alphanumeric(c))
			{
				lowercase_word.push_back(lowercase(c));
				state = State::word_wait_delimiter;
			}
			else if (c == '*')
			{
				characteristics.set(MorphologyCharacteristic::deprecated, true);
			}
			else if (c == '\n')
			{
				group++;
				state = State::double_endline_wait_word;
			}
			break;
		}
		
		case State::word_wait_delimiter:
		{
			if (is_alphanumeric(c) || c == '-')
			{
				lowercase_word.push_back(lowercase(c));
			}
			else if (c == '|')
			{
				state = State::space_wait_characteristic;
			}
			else
			{
				state = State::word_space_wait_delimiter;
			}
			break;
		}
		
		case State::word_space_wait_delimiter:
		{
			if (is_alphanumeric(c))
			{
				lowercase_word.push_back(' ');
				lowercase_word.push_back(lowercase(c));
				state = State::word_wait_delimiter;
			}
			else if (c == '|')
			{
				state = State::space_wait_characteristic;
			}
			break;
		}
		
		case State::space_wait_characteristic:
		{
			if (is_alphanumeric(c))
			{
				text_characteristic.push_back(lowercase(c));
				state = State::characteristic_wait_space_delimiter;
			}
			else if (c == '|')
			{
				_parse_morphology_add(group, lowercase_word, characteristics);
				characteristics = MorphologyCharacteristics();
				lowercase_word.resize(0);
				state = State::wait_endline;
			}
			break;
		}
		
		case State::characteristic_wait_space_delimiter:
		{
			if (is_alphanumeric(c) || c == '-')
			{
				text_characteristic.push_back(lowercase(c));
			}
			else if (c == ' ' || c == '|')
			{
				bool found = false;
				for (unsigned int i = 0; i < text_characteristics.size(); i++)
				{
					if (text_characteristic == text_characteristics[i])
						{ characteristics.set((MorphologyCharacteristic)i, true); found = true; break; }
				}
				if (!found)
				{
					text_characteristics.push_back(text_characteristic);
					characteristics.set((MorphologyCharacteristic)(text_characteristics.size() - 1), true);
				}

				text_characteristic.resize(0);
				if (c == ' ')
				{
					state = State::space_wait_characteristic;
				}
				else
				{
					_parse_morphology_add(group, lowercase_word, characteristics);
					characteristics = MorphologyCharacteristics();
					lowercase_word.resize(0);
					state = State::wait_endline;
				}
			}
			break;
		}

		case State::wait_endline:
		{
			if (c == '\n') state = State::endline_wait_word;
			break;
		}
		}

		//Report success
		position++;
		if ((unsigned int)(100.0 * position / morphology_size) > reported)
		{
			reported = (unsigned int)(100.0 * position / morphology_size);
			char buffer[64];
			sprintf(buffer, "Morphology file parsing %u", reported);
			_mechan->print_event_log(buffer);
		}
	}
	return true;
}

void mechan::Word::_parse_synonym_add(unsigned int group, const std::string lowercase_word) noexcept
{
	if (lowercase_word.empty() || _unpacked_words.count(lowercase_word) == 0) return;

	_unpacked_words[lowercase_word].synonym_groups.insert(group);
	for (auto i = _unpacked_words[lowercase_word].morphology_characteristics.cbegin();
		i != _unpacked_words[lowercase_word].morphology_characteristics.cend();
		i++)
	{
		unsigned int morphology_group = i->first;
		for (auto j = _unpacked_morphology_groups[morphology_group].lowercase_words.cbegin();
			j != _unpacked_morphology_groups[morphology_group].lowercase_words.cend();
			j++)
		{
			const std::string lowercase_conjugated_word = *j;
			_unpacked_words[lowercase_conjugated_word].synonym_groups.insert(group);
		}
	}
}

bool mechan::Word::_parse_synonym() noexcept
{
	enum class State
	{
		delimiter_wait_word,
		word_wait_delimiter,
		word_space_wait_delimiter,
		invalid_wait_delimiter
	};
	
	//Open file
	ir::FileResource synonym = fopen(MECHAN_DIR "\\data\\synonym.txt", "r");
	if (synonym == nullptr) return false;
	_mechan->print_event_log("Synonym file found");
	fseek(synonym, 0, SEEK_END);
	unsigned int synonym_size = ftell(synonym);
	fseek(synonym, 0, SEEK_SET);

	//Init variables
	State  state = State::delimiter_wait_word;
	std::string lowercase_word;
	unsigned int group = 0;
	unsigned int reported = 0;
	unsigned int position = 0;

	//Parse
	while (true)
	{
		//Read and process symbol
		char c;
		if (fread(&c, 1, 1, synonym) == 0)
		{
			_parse_synonym_add(group, lowercase_word);
			break;
		}
		else switch (state)
		{
		case State::delimiter_wait_word:
		{
			if (is_alphanumeric(c))
			{
				lowercase_word.push_back(lowercase(c));
				state = State::word_wait_delimiter;
			}
			break;
		}

		case State::word_wait_delimiter:
		{
			if (is_alphanumeric(c) || c == '-')
			{
				lowercase_word.push_back(lowercase(c));
			}
			else if (c == ' ')
			{
				state = State::word_space_wait_delimiter;
			}
			else if (c == '|')
			{
				_parse_synonym_add(group, lowercase_word);
				lowercase_word.resize(0);
				state = State::delimiter_wait_word;
			}
			else if (c == '\n')
			{
				_parse_synonym_add(group, lowercase_word);
				lowercase_word.resize(0);
				group++;
				state = State::delimiter_wait_word;
			}
			else
			{
				_parse_synonym_add(group, lowercase_word);
				lowercase_word.resize(0);
				state = State::invalid_wait_delimiter;
			}
			break;
		}

		case State::word_space_wait_delimiter:
		{
			if (is_alphanumeric(c))
			{
				lowercase_word.push_back(' ');
				lowercase_word.push_back(lowercase(c));
				state = State::word_wait_delimiter;
			}
			else if (c == '|')
			{
				_parse_synonym_add(group, lowercase_word);
				lowercase_word.resize(0);
				state = State::delimiter_wait_word;
			}
			else if (c == '\n')
			{
				_parse_synonym_add(group, lowercase_word);
				lowercase_word.resize(0);
				group++;
				state = State::delimiter_wait_word;
			}
			else
			{
				_parse_synonym_add(group, lowercase_word);
				lowercase_word.resize(0);
				state = State::invalid_wait_delimiter;
			}
			break;
		}

		case State::invalid_wait_delimiter:
		{
			if (c == '|')
			{
				state = State::delimiter_wait_word;
			}
			else if (c == '\n')
			{
				group++;
				state = State::delimiter_wait_word;
			}
			break;
		}
		}

		//Report success
		position++;
		if ((unsigned int)(100.0 * position / synonym_size) > reported)
		{
			reported = (unsigned int)(100.0 * position / synonym_size);
			char buffer[64];
			sprintf(buffer, "Synonym file parsing %u", reported);
			_mechan->print_event_log(buffer);
		}
	}
	return true;
}

unsigned int mechan::Word::_pack_morphology_group_occurencies(unsigned int group) const noexcept
{
	unsigned int occurencies = 0;
	for (auto i = _unpacked_morphology_groups[group].lowercase_words.cbegin();
		i != _unpacked_morphology_groups[group].lowercase_words.cend();
		i++)
	{
		occurencies += _unpacked_words.at(*i).lowercase_occurence_number;
		occurencies += _unpacked_words.at(*i).uppercase_occurence_number;
	}
	return occurencies;
}

bool mechan::Word::_pack() noexcept
{
	_words = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\words", ir::Database::create_mode::neww, nullptr);
	if (!_words->ok() || _words->set_ram_mode(true, true) != ir::ec::ok)
	{
		delete _words;
		_words = nullptr;
		return false;
	}

	for (auto i = _unpacked_words.cbegin(); i != _unpacked_words.cend(); i++)
	{
		//Find group/characteristics with most occurencies
		unsigned int max_occurencies = 0;
		MorphologyCharacteristics max_characteristics;
		for (auto j = i->second.morphology_characteristics.cbegin();
			j != i->second.morphology_characteristics.cend();
			j++)
		{
			unsigned int new_occurencies = _pack_morphology_group_occurencies(j->first);
			if (new_occurencies > max_occurencies)
			{
				max_occurencies = new_occurencies;
				max_characteristics = j->second;
			}
		}

		//Add to database
		const std::string lowercase_word = i->first;
		unsigned int morphology_group_number = (unsigned int)i->second.morphology_characteristics.size();
		unsigned int synonym_group_number = (unsigned int)i->second.synonym_groups.size();
		_buffer.resize(sizeof(WordInfo) + (morphology_group_number + synonym_group_number) * sizeof(unsigned int));
		WordInfo *word_info = (WordInfo*)&_buffer[0];
		word_info->lowercase_occurence_number = i->second.lowercase_occurence_number;
		word_info->uppercase_occurence_number = i->second.uppercase_occurence_number;
		word_info->morphology_group_number = morphology_group_number;
		word_info->probable_characteristics = max_characteristics;
		unsigned int k = 0;
		for (auto j = i->second.morphology_characteristics.cbegin(); j != i->second.morphology_characteristics.cend(); j++, k++)
			word_info->morphology_groups()[k] = j->first;
		k = 0;
		for (auto j = i->second.synonym_groups.cbegin(); j != i->second.synonym_groups.cend(); j++, k++)
			word_info->morphology_groups()[k] = *j;
		ir::ConstBlock key(lowercase_word.data(), lowercase_word.size());
		ir::ConstBlock data(_buffer.data(), _buffer.size());
		if (_words->insert(key, data) != ir::ec::ok)
		{
			delete _words;
			_words = nullptr;
			return false;
		}
	}

	delete _words;
	_words = new ir::S2STDatabase(WIDE_MECHAN_DIR "\\data\\words", ir::Database::create_mode::read, nullptr);
	if (!_words->ok() || _words->set_ram_mode(true, true) != ir::ec::ok)
	{
		delete _words;
		_words = nullptr;
		return false;
	}

	return true;
}

mechan::Word::Word(Mechan *mechan) noexcept : _mechan(mechan)
{
	if (!_load())
	{
		if (_parse_dialog() && _parse_morphology() && _parse_synonym()) _pack();
		_unpacked_words.clear();
		_unpacked_morphology_groups.clear();
	}
}

bool mechan::Word::ok() const noexcept
{
	return _words != nullptr && _words->ok();
}

bool mechan::Word::word_info(std::string lowercase_word, const WordInfo **info, unsigned int *info_size) const noexcept
{
	assert(ok());
	ir::ConstBlock key(lowercase_word.data(), lowercase_word.size());
	ir::ConstBlock data;
	if (_words->read(key, &data) != ir::ec::ok) return false;
	*info_size = data.size();
	*info = (WordInfo*)data.data();
	return true;
}

mechan::Word::~Word() noexcept
{
	if (_words != nullptr) delete _words;
}