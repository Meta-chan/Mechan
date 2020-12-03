#include "../header/mechan_lowercase.h"

std::string mechan::lowercase(const std::string string)
{
	std::string s = string;
	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i] >= 'A' && s[i] <= 'Z') s[i] = s[i] + 'a' - 'A';					//A - Z
		else if (s[i] >= 0xC0 && s[i] <= 0xDF) s[i] = s[i] + (char)(0xE0 - 0xC0);	//À - ß
		else if (s[i] == 0xA8 || s[i] == 0xB8) s[i] = (char)0xE5;					//¨
		else if (s[i] == 0xAA) s[i] = (char)0xBA;									//ª
		else if (s[i] == 0xAF) s[i] = (char)0xBF;									//¯
		else if (s[i] == 0xB2) s[i] = (char)0xB3;									//²
		else if (s[i] == 0xA5) s[i] = (char)0xB4;									//Ã
	}
	return s;
}