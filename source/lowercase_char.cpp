#include "../header/lowercase_char.h"

char lowercase_char(char c, bool *isletter)
{
	if (c == '-') {}
	else if (c >= '0' && c <= '9') {}
	else if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
	else if (c >= 'a' && c <= 'z') {}
	else if (((unsigned char)c) == 0xA8) c = (char)0xE5;
	else if (((unsigned char)c) == 0xB8) c = (char)0xE5;
	else if (((unsigned char)c) >= 0xC0 && ((unsigned char)c) < 0xE0) c = (char)(((unsigned char)c) + 0x20);
	else if (((unsigned char)c) >= 0xE0) {}
	else { if (isletter != nullptr) *isletter = false; return c; }

	if (isletter != nullptr) *isletter = true;
	return c;
};