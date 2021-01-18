#include "../header/mechan_character.h"

bool mechan::is_latin(char c) noexcept
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool mechan::is_cyrylic(char c) noexcept
{
	unsigned char u = (unsigned char)c;
	return u == 0xA8 || u == 0xB8 || u >= 0xC0;
}

bool mechan::is_uppercase(char c) noexcept
{
	unsigned char u = (unsigned char)c;
	return (u >= 'A' && u <= 'Z') || (u >= 0xC0 && u <= 0xDF) || u == 0xA8 || u == 0xAA || u == 0xAF || u == 0xB2 || u == 0xA5;
}

bool mechan::is_alphanumeric(char c) noexcept
{
	unsigned char u = (unsigned char)c;
	return
		(c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		(u >= 0xC0) || u == 0xA8 || u == 0xAA || u == 0xAF || u == 0xB2 || u == 0xA5;
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