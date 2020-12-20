#pragma once

#include <string>
#include <vector>

namespace mechan
{
	void parse(const std::string string, std::vector<std::string> *words, bool lower) noexcept;
}