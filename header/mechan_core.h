#pragma once

#include "mechan_interface.h"
#include <string>
#include <random>

namespace mechan
{
	class Core
	{
	private:
		bool _ok = false;
		bool _reqest_shutdown = false;
		std::uniform_int_distribution<unsigned int> _distribution;
		std::default_random_engine _engine;

		bool _intersect(const std::vector<unsigned int> *a, const std::vector<unsigned int> *b) const noexcept;

	public:
		Core() noexcept;
		std::string answer(Interface::Address address, const std::string question)	noexcept;
		bool request_shutdown()														const noexcept;
		bool ok()																	const noexcept;
	};
}