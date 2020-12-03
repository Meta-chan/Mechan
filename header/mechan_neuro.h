#pragma once

#include <ir_neuro.h>
#include <string>
#include <random>

namespace mechan
{
	class Neuro
	{
	private:
		static const unsigned int n_words = 10;
		static const unsigned int n_chars = 10;
		static const unsigned int message_size = 33 * n_chars * n_words + 3;
		static const unsigned int negative_pro_positive = 1;
		static const double train_part;
		static const double coefficient;

		std::default_random_engine _generator;
		std::uniform_int_distribution<unsigned int> *_distribution	= nullptr;
		ir::Neuro<double> *_neuro									= nullptr;

		const char *_unroll_word(const char *begin, double v[33 * n_chars]);
		void _unroll_message(const std::string message, double v[message_size]) noexcept;

	public:
		Neuro()																		noexcept;
		bool ok()																	const noexcept;
		void train()																noexcept;
		double qestion_answer(const std::string question, const std::string answer)	noexcept;
		~Neuro()																	noexcept;
	};
}