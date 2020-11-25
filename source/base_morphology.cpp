#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define IR_IMPLEMENT
#include <ir_utf.h>
#include <ir_database/ir_s2st_database.h>
#include <ir_database/ir_n2st_database.h>

class MorphologyParser
{
private:
	char _symbol							= '\0';
	bool _end								= false;
	FILE *_sourcefile						= nullptr;
	FILE *_descriptionfile					= nullptr;
	ir::S2STDatabase *_word2group			= nullptr;
	ir::N2STDatabase *_group2data			= nullptr;
	unsigned int _groupcount				= 0;
	std::vector<unsigned char> _groupdata;
	std::vector<char*> _descriptions;

	void _skipspace();
	void _skipspacedelim();
	void _writeword(bool addtobase, unsigned int *maxlength);
	void _writedescription();
	void _skipendline(bool *endblock);

public:
	MorphologyParser(const wchar_t *sourcepath, const wchar_t *descriptionpath, const wchar_t *word2grouppath, const wchar_t *group2datapath, ir::ec *code);
	void parse();
	~MorphologyParser();
};

void MorphologyParser::_skipspace()
{
	if (_end) return;

	while(_symbol == ' ')
	{
		size_t read = fread(&_symbol, sizeof(char), 1, _sourcefile);
		if (read == 0) { _end = true; break; }		
	};
};

void MorphologyParser::_skipspacedelim()
{
	if (_end) return;

	while ((_symbol == ' ' || _symbol == '|'))
	{
		size_t read = fread(&_symbol, sizeof(char), 1, _sourcefile);
		if (read == 0) { _end = true; break; }
	};
};

void MorphologyParser::_writeword(bool addtobase, unsigned int *maxlength)
{
	if (_end) return;

	//Чтение слова
	unsigned int count = 0;
	char word[256];
	while(_symbol != '|')
	{
		word[count] = _symbol;
		count++;
		size_t read = fread(&_symbol, sizeof(char), 1, _sourcefile);
		if (read == 0) { _end = true; break; }
	};
	
	while (word[count - 1] == ' ') count--;
	if ((maxlength != nullptr) && (count > *maxlength)) *maxlength = count;

	//Запись слова в базу в исходном виде
	for (unsigned int i = 0; i < count; i++) _groupdata.push_back(word[i]);

	//Если слово пишется в базу слов - нижний регистр и замена ё на е:
	if (addtobase)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			if (word[i] >= 'A' && word[i] <= 'Z') word[i] = word[i] + 'a' - 'A';			//A - Z
			else if (word[i] >= 0xC0 && word[i] <= 0xDF) word[i] = word[i] + (0xE0 - 0xC0);	//А - Я
			else if (word[i] == 0xA8 || word[i] == 0xB8) word[i] = 0xE5;					//Ё
			else if (word[i] == 0xAA) word[i] = 0xBA;										//Є
			else if (word[i] == 0xAF) word[i] = 0xBF;										//Ї
			else if (word[i] == 0xB2) word[i] = 0xB3;										//І
			else if (word[i] == 0xA5) word[i] = 0xB4;										//Г
		}

		ir::ConstBlock key(count, &word[0]);
		
		
	}
	
	_groupdata.push_back('\0');
};

void MorphologyParser::_writedescription()
{
	if (_end) return;

	char textdesc[256];
	unsigned int textdesclen = 0;

	while (_symbol != ' ' && _symbol != '|')
	{
		textdesc[textdesclen] = _symbol;
		textdesclen++;
		size_t read = fread(&_symbol, sizeof(char), 1, _sourcefile);
		if (read == 0) { _end = true; break; }
	}

	textdesc[textdesclen] = '\0';
	bool match = false;
	for (unsigned int j = 0; j < _descriptions.size(); j++)
	{
		if (strcmp(_descriptions[j], textdesc) == 0)
		{
			match = true;
			_groupdata.push_back(j + 1);
		}
	}

	if (!match)
	{
		int bytedesc = _descriptions.size() + 1;
		char *textdesccopy = _strdup(textdesc);
		_groupdata.push_back(bytedesc);
		fprintf(_descriptionfile, "%i = %s\n", bytedesc, textdesc);
		_descriptions.push_back(textdesccopy);
	}
}

void MorphologyParser::_skipendline(bool *endblock)
{
	if (_end) return;

	while (_symbol != '\r' && _symbol != '\n')
	{
		size_t read = fread(&_symbol, sizeof(char), 1, _sourcefile);
		if (read == 0) { _end = true; break; }
	};

	unsigned int endlines = 0;
	while (_symbol == '\r' || _symbol == '\n' || _symbol == ' ' || _symbol == '|')
	{
		if (_symbol == '\n') endlines++;
		size_t read = fread(&_symbol, sizeof(char), 1, _sourcefile);
		if (read == 0) { _end = true; break; }
	};

	*endblock = (endlines > 1);
}

MorphologyParser::MorphologyParser(const wchar_t *sourcepath, const wchar_t *descriptionpath, const wchar_t *word2grouppath, const wchar_t *group2datapath, ir::ec *code)
{
	_sourcefile = _wfopen(sourcepath, L"r");
	if (_sourcefile == nullptr) *code = ir::ec::ec_other;

	_descriptionfile = _wfopen(descriptionpath, L"w");
	if (_descriptionfile == nullptr) *code = ir::ec::ec_other;

	_word2group = new ir::S2STDatabase(word2grouppath, ir::Database::create_new_always, code);
	if (*code != ir::ec::ec_ok) return;

	_group2data = new ir::N2STDatabase(group2datapath, ir::Database::create_new_always, code);
	if (*code != ir::ec::ec_ok) return;

	*code = ir::ec::ec_ok;
};

MorphologyParser::~MorphologyParser()
{
	delete _group2data;
	delete _word2group;
	if (_sourcefile != nullptr) fclose(_sourcefile);
	if (_descriptionfile != nullptr) fclose(_descriptionfile);
	for (unsigned int i = 0; i < _descriptions.size(); i++) free(_descriptions[i]);
};

void MorphologyParser::parse()
{
	
};

int persemorphology()
{
	const wchar_t *sourcepath = L"C:\\Project\\_source\\mechan\\morphology.txt";
	const wchar_t *descriptionpath = L"C:\\Project\\_source\\mechan\\morphology_description.txt";
	const wchar_t *word2grouppath = L"C:\\Project\\mechan\\database\\word2group.idt";
	const wchar_t *group2datapath = L"C:\\Project\\mechan\\database\\group2data.idt";

	_wunlink(word2grouppath);
	_wunlink(group2datapath);

	ir::ec code;
	MorphologyParser parser(sourcepath, descriptionpath, word2grouppath, group2datapath, &code);
	if (code == 0)
	{
		parser.parse();
		getchar();
	}
	return code;
};

int main()
{
	persemorphology();
};