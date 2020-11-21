#define IR_IMPLEMENT
#include <ir_database/ir_s2st_database.h>
#include <ir_database/ir_n2st_database.h>

void lowercase(char *s, bool *uppercase, bool *yo)
{
	while (true)
	{
		if (*s >= 'A' && *s <= 'Z')
		{
			*s = *s + ('a' - 'A');
			if (uppercase != nullptr) *uppercase = true;
		}
		else if (((unsigned char)*s) == 0xA8)
		{
			*s = (char)0xE5;
			if (uppercase != nullptr) *uppercase = true;
			if (yo != nullptr) *yo = true;
		}
		else if (((unsigned char)*s) == 0xB8)
		{
			*s = (char)0xE5;
			if (yo != nullptr) *yo = true;
		}
		else if (((unsigned char)*s) >= 0xC0 && ((unsigned char)*s) < 0xE0)
		{
			*s = (char)(((unsigned char)*s) + 0x20);
			if (uppercase != nullptr) *uppercase = true;
		}
		else if (*s == '\0') break;
		s++;
	}
};

void trim(char* s)
{
	unsigned int firsti = 0;
	unsigned int lasti = strlen(s);
	while (firsti < lasti && s[firsti] == ' ') firsti++;
	while (firsti < lasti && s[lasti - 1] == ' ') lasti--;
	for (unsigned int i = 0; i < (lasti - firsti); i++) s[i] = s[i + firsti];
	s[lasti - firsti] = '\0';
};

int _word2syn()
{
	FILE *synmaster = _wfsopen(L"C:\\Project\\_source\\Mechan\\synmaster.txt", L"rb", _SH_DENYNO);
	if (synmaster == nullptr) return 1;

	ir::ec code;
	ir::S2STDatabase word2syn(L"C:\\Project\\mechan\\database\\word2syn", ir::Database::create_new_always, &code);
	if (code != ir::ec::ec_ok) return 2;

	char word[1024];
	unsigned int group = 0;
	unsigned int i = 0;
	while (true)
	{
		char c;
		unsigned int read = fread(&c, sizeof(char), 1, synmaster);
		if (read == 0 || c == '|' || c == '\n' || c == '\r')
		{
			if (i != 0)
			{
				word[i] = '\0';
				lowercase(word, nullptr, nullptr);
				trim(word);
				word2syn.insert(ir::ConstBlock(i, word), ir::ConstBlock(sizeof(unsigned int), &group));
				if (c == '\n' || c == '\r') group++;
				i = 0;
			}
			if (read == 0) break;
		}
		else
		{
			word[i] = c;
			i++;
		}
	}
	return 0;
};

int _group2syn()
{
	FILE *synmaster = _wfsopen(L"C:\\Project\\_source\\Mechan\\synmaster.txt", L"rb", _SH_DENYNO);
	if (synmaster == nullptr) return 1;

	ir::ec code;
	ir::S2STDatabase word2syn(L"C:\\Project\\mechan\\database\\word2syn", ir::Database::create_new_always, &code);
	if (code != ir::ec::ec_ok) return 2;

	char word[1024];
	unsigned int group = 0;
	unsigned int i = 0;
	while (true)
	{
		char c;
		unsigned int read = fread(&c, sizeof(char), 1, synmaster);
		if (read == 0 || c == '|' || c == '\n' || c == '\r')
		{
			if (i != 0)
			{
				word[i] = '\0';
				lowercase(word, nullptr, nullptr);
				trim(word);
				word2syn.insert(ir::ConstBlock(i, word), ir::ConstBlock(sizeof(unsigned int), &group));
				if (c == '\n' || c == '\r') group++;
				i = 0;
			}
			if (read == 0) break;
		}
		else
		{
			word[i] = c;
			i++;
		}
	}
	return 0;
};

int main()
{
	return _word2syn();
};