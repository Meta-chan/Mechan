#include "../header/mechan_parse.h"
#include "../header/mechan_character.h"

void mechan::parse_punctuation(const std::string string, Parsed *parsed) noexcept
{
	parsed->words.resize(0);
	bool request_new = true;
	for (unsigned int i = 0; i < string.size(); i++)
	{
		if((string[i] >= '0' && string[i] <= '9')
		|| (string[i] >= 'A' && string[i] <= 'z')
		|| ((unsigned char)string[i]) == 0xA8		//¨
		|| ((unsigned char)string[i]) == 0xB8		//¸
		|| ((unsigned char)string[i]) >= 0xC0)		//À - ÿ
		{
			if (request_new)
			{
				Parsed::Word new_word;
				new_word.uppercase = is_uppercase(string[i]);
				parsed->words.push_back(new_word);
				request_new = false;
			}
			parsed->words.back().lowercase.push_back(lowercase(string[i]));
		}
		else if (string[i] == '-')
		{
			if (!request_new) parsed->words.back().lowercase.push_back(string[i]);
		}
		else request_new = true;
	}
}

void mechan::parse_space(const std::string string, std::vector<std::string> *words) noexcept
{
	words->resize(0);
	bool request_new = true;
	for (unsigned int i = 0; i < string.size(); i++)
	{
		if (string[i] == ' ' || string[i] == ',')
		{
			request_new = true;
		}
		else if ((string[i] == '!' || string[i] == '?') && i == 0)
		{
		}
		else
		{
			if (request_new)
			{
				words->push_back(std::string());
				request_new = false;
			}
			words->back().push_back(lowercase(string[i]));
		}
	}
}