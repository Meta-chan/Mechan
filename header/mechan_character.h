#pragma once

#include <string>

namespace mechan
{
	static const char ellipsis = (char)0x85;
	static const char ndash = (char)0x96;
	static const char mdash = (char)0x97;
	static const char empty1 = (char)0xA0;
	static const char empty2 = (char)0xAD;
	static const char empty3 = (char)0x98;
	static const char quoteopen = (char)0xAB;
	static const char quoteclose = (char)0xBB;

	bool is_latin(char c)							noexcept;
	bool is_cyrylic(char c)							noexcept;
	bool is_uppercase(char c)						noexcept;
	bool is_alphanumeric(char c)					noexcept;
	char lowercase(char c)							noexcept;
	std::string lowercase(const std::string string)	noexcept;
}