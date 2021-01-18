#pragma once

#include <string>
#include <vector>

namespace mechan
{
	struct Parsed
	{
		struct Word
		{
			std::string lowercase;
			bool uppercase;
		};
		
		std::vector<Word> words;
		char end;
	};

	void parse_punctuation(const std::string string, Parsed *parsed) noexcept;
	void parse_space(const std::string string, std::vector<std::string> *words) noexcept;
}