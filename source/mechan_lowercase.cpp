#include "../header/mechan_lowercase.h"

char mechan::lowercase(char c) noexcept
{
	if (c >= 'A' && c <= 'Z') return c + 'a' - 'A';						//A - Z
	else if (c >= 0xC0 && c <= 0xDF) return c + (char)(0xE0 - 0xC0);	//� - �
	else if (c == 0xA8 || c == 0xB8) return (char)0xE5;					//�
	else if (c == 0xAA) return (char)0xBA;								//�
	else if (c == 0xAF) return (char)0xBF;								//�
	else if (c == 0xB2) return (char)0xB3;								//�
	else if (c == 0xA5) return (char)0xB4;								//�
	else return c;
}

std::string mechan::lowercase(const std::string string) noexcept
{
	std::string s = string;
	for (size_t i = 0; i < s.size(); i++)
	{
		s[i] = lowercase(string[i]);
	}
	return s;
}