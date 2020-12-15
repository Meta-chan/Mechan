#pragma once

#include <string>

namespace mechan
{
	bool is_lowercase_alphanumeric(char c) noexcept;
	bool is_lowercase(char c) noexcept;
	char lowercase(char c) noexcept;
	std::string lowercase(const std::string string) noexcept;
}