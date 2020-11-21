#include "../header/lowercase.h"

void lowercase(std::string *s, bool *uppercase, bool *yo)
{
	unsigned int len = s->size();
	for (unsigned int i = 0; i < len; i++)
	{
		char c = s->at(i);
		if (c >= 'A' && c <= 'Z')
		{
			s->at(i) = c + ('a' - 'A');
			if (uppercase != nullptr) *uppercase = true;
		}
		else if (((unsigned char)c) == 0xA8)
		{
			s->at(i) = (char)0xE5;
			if (uppercase != nullptr) *uppercase = true;
			if (yo != nullptr) *yo = true;
		}
		else if (((unsigned char)c) == 0xB8)
		{
			s->at(i) = (char)0xE5;
			if (yo != nullptr) *yo = true;
		}
		else if (((unsigned char)c) >= 0xC0 && ((unsigned char)c) < 0xE0)
		{
			s->at(i) = (char)(((unsigned char)c) + 0x20);
			if (uppercase != nullptr) *uppercase = true;
		}
	}
};