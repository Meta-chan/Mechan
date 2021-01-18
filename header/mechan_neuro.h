#pragma once

#include "mechan_parse.h"

#include <string>
#include <random>
#include <time.h>
#include <ir_neuro.h>

namespace mechan
{
	class Mechan;

	class Neuro
	{
	private:
		static const unsigned int n_words = 8;
		static const unsigned int n_chars = 8;
		static const unsigned int message_size = 33 * n_chars * n_words + 3;
		static const unsigned int negative_pro_positive = 1;
		
		Mechan *_mechan												= nullptr;
		ir::Neuro<double> *_neuro									= nullptr;
		clock_t _last_save											= 0;
		std::default_random_engine _generator;
		std::uniform_int_distribution<unsigned int> _distribution;

		void _unroll_char(char c, double v[33])										noexcept;
		void _unroll_word(const std::string lowercase_word, double v[33 * n_chars])	noexcept;
		void _unroll_message(const Parsed *message, double v[message_size])			noexcept;

	public:
		Neuro(Mechan *mechan)												noexcept;
		bool ok()															const noexcept;
		double get_coefficient()											const noexcept;
		void set_coefficient(double coefficient)							noexcept;
		void save()															noexcept;
		void train()														noexcept;
		double qestion_answer(const Parsed *question, const Parsed *answer)	noexcept;
		~Neuro()															noexcept;
	};
}