#pragma once

#include <string>
#include <random>

namespace mechan
{
	class Mechan;

	class Core
	{
	private:
		Mechan *_mechan	= nullptr;
		bool _ok		= false;

		bool _intersect(const std::vector<unsigned int> *a, const std::vector<unsigned int> *b) const noexcept;

	public:
		Core(Mechan *mechan)							noexcept;
		std::string answer(const std::string question)	noexcept;
		bool ok()										const noexcept;
	};
}