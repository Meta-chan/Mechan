#pragma once

#include "mechan_dialog.h"
#include "mechan_word.h"
#include "mechan_neuro.h"
#include <string>
#include <random>

namespace mechan
{
	class Core
	{
	public:
		static const size_t timeout = 10;
		static const size_t best_size = 10;

	private:
		bool _ok		= false;
		Dialog *_dialog	= nullptr;
		Word *_word		= nullptr;
		Neuro *_neuro	= nullptr;

		bool _intersect(const std::vector<size_t> *a, const unsigned int *b, size_t bsize) const noexcept;

	public:
		Core(Dialog *dialog, Word *word)				noexcept;
		std::string answer(const std::string question)	noexcept;
		bool ok()										const noexcept;
	};
}