#pragma once

#include <string>
#include <vector>

namespace mechan
{
	void parse_punctuation(const std::string string, std::vector<std::string> *words) noexcept;
	void parse_space(const std::string string, std::vector<std::string> *words) noexcept;
}