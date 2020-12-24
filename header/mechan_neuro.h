#pragma once

#include <string>
#include <random>
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
		std::default_random_engine _generator;
		std::uniform_int_distribution<unsigned int> *_distribution	= nullptr;
		ir::Neuro<double> *_neuro									= nullptr;
		
		void _unroll_char(char c, double v[33])								noexcept;
		void _unroll_word(const std::string word, double v[33 * n_chars])	noexcept;
		void _unroll_message(
			const std::vector<std::string> *message, char message_type,
			double v[message_size])											noexcept;

	public:
		Neuro(Mechan *mechan)												noexcept;
		bool ok()															const noexcept;
		void save()															const noexcept;
		void train()														noexcept;
		double qestion_answer(
			const std::vector<std::string> *question, char question_type,
			const std::vector<std::string> *answer, char answer_type)		noexcept;
		~Neuro()															noexcept;
	};
}