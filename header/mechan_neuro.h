#pragma once

#include "mechan_parse.h"
#include "mechan_dialog.h"
#include "mechan_word.h"

#include <neurog/neurog.h>
#include <string>
#include <random>
#include <time.h>

namespace mechan
{
	class Neuro
	{
	public:
		static const size_t n_words = 10;
		static const size_t n_chars = 10;
		static const size_t alphabet_size = 33;
		static const size_t message_size = alphabet_size * n_chars * n_words + 3;
		static const size_t batch_size = 1024;
		static const size_t interval = 3600;
		static const float train_part;
		static const float test_part;
		static const float coefficient;
		static const float deviance;

	private:
		neurog::Neuro _neuro;
		Dialog *_dialog					= nullptr;
		Word *_word						= nullptr;
		unsigned int _currect_message	= 0;
		unsigned int _currect_progress	= 0;
		unsigned int _test_count		= 0;
		unsigned int _match_count		= 0;
		clock_t _last_save				= 0;
		std::default_random_engine _generator;
		std::uniform_int_distribution<unsigned int> _distribution;
		ir::QuietVector<float> _input_buffer;
		ir::QuietVector<float> _output_buffer;

		void _unroll_char(char c, float v[alphabet_size])										noexcept;
		void _unroll_word(const std::string lowercase_word, float v[alphabet_size * n_chars])	noexcept;
		void _unroll_message(const Parsed *message, float v[message_size])						noexcept;
		bool _train()																			noexcept;
		bool _test()																			noexcept;

	public:
		Neuro(Dialog *dialog, Word *word, bool train)									noexcept;
		bool ok()																		const noexcept;
		float get_coefficient()															const noexcept;
		void set_coefficient(float coefficient)											noexcept;
		bool save()																		noexcept;
		bool train()																	noexcept;
		bool store_messages(GLint sample, const Parsed *question, const Parsed *answer)	noexcept;
		bool load_heuristics(GLint batch_size, float ** const data)						noexcept;
		~Neuro()																		noexcept;
	};
}