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
		static int word_number;
		static int char_number;
		static int char_size;
		static int interval;
		static int batch_size;
		static bool batch_correct;
		static int layer2_size;
		static int layer3_size;
		static float train_part;
		static float test_part;
		static float coefficient;
		static float deviance;

		static int word_size;
		static int message_size;

	private:
		enum class State
		{
			begin_testing,
			train_testing,
			test_testing,
			begin_training,
			training
		};

		neurog::Neuro _neuro;
		Dialog *_dialog					= nullptr;
		Word *_word						= nullptr;
		unsigned int _currect_message	= 0;
		unsigned int _currect_progress	= 0;
		unsigned int _train_pair_count	= 0;
		float _train_cost_count			= 0.0f;
		unsigned int _train_guess_count	= 0;
		unsigned int _test_pair_count	= 0;
		float _test_cost_count			= 0.0f;
		unsigned int _test_guess_count	= 0;
		clock_t _last_save				= 0;
		State _state					= State::begin_testing;
		std::default_random_engine _generator;
		std::uniform_int_distribution<unsigned int> _distribution;
		ir::QuietVector<float> _input_buffer;
		ir::QuietVector<float> _output_buffer;

		void _unroll_char(char c, float *v)								noexcept;
		void _unroll_word(const std::string lowercase_word, float *v)	noexcept;
		void _unroll_message(const Parsed *message, float *v)			noexcept;
		bool _train()													noexcept;
		bool _test()													noexcept;

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