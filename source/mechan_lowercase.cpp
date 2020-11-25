#include "../header/mechan_lowercase.h"

void mechan::lowercase(char *string)
{
	while (*string != '\0')
	{
		if (*string >= 'A' && *string <= 'Z') *string = *string + 'a' - 'A';					//A - Z
		else if (*string >= 0xC0 && *string <= 0xDF) *string = *string + (char)(0xE0 - 0xC0);	//À - ß
		else if (*string == 0xA8 || *string == 0xB8) *string = (char)0xE5;						//¨
		else if (*string == 0xAA) *string = (char)0xBA;											//ª
		else if (*string == 0xAF) *string = (char)0xBF;											//¯
		else if (*string == 0xB2) *string = (char)0xB3;											//²
		else if (*string == 0xA5) *string = (char)0xB4;											//Ã
	}
}