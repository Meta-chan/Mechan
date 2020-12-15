#include "../header/mechan_lowercase.h"

bool mechan::is_lowercase_alphanumeric(char c) noexcept
{
	return ((c >= '0' && c <= '9') || (c == '-') || ((unsigned char)c >= (unsigned char)0xE0));
}

bool mechan::is_lowercase(char c) noexcept
{
	return ((unsigned char )c >= (unsigned char)0xE0);
}

char mechan::lowercase(char c) noexcept
{
	unsigned char u = (unsigned char)c;
	if (u >= 'A' && u <= 'Z') return (char)(u + 'a' - 'A');					//A - Z
	else if (u >= 0xC0 && u <= 0xDF) return (char)(u + (char)(0xE0 - 0xC0));//À - ß
	else if (u == 0xA8 || u == 0xB8) return (char)0xE5;						//¨
	else if (u == 0xAA) return (char)0xBA;									//ª
	else if (u == 0xAF) return (char)0xBF;									//¯
	else if (u == 0xB2) return (char)0xB3;									//²
	else if (u == 0xA5) return (char)0xB4;									//Ã
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