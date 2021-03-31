#define IR_INCLUDE 'i'
#define MATHG_INCLUDE
#define NEUROG_INCLUDE
#include "../header/mechan_neuro.h"
#include "../header/mechan_parse.h"
#include "../header/mechan_character.h"
#include <mathg/default.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

const float mechan::Neuro::train_part = 0.7f;
const float mechan::Neuro::test_part = 0.3f;
const float mechan::Neuro::coefficient = 0.001f;
const float mechan::Neuro::deviance = 0.01f;

void mechan::Neuro::_unroll_char(char c, float v[alphabet_size]) noexcept
{
	size_t nchar = is_cyrylic(c) ? ((unsigned char)c - (unsigned char)0xE0) : 32;
	for (size_t i = 0; i < nchar; i++) v[i] = 0.0f;
	v[nchar] = 1.0f;
	for (size_t i = nchar + 1; i < alphabet_size; i++) v[i] = 0.0f;
}

void mechan::Neuro::_unroll_word(const std::string word, float v[alphabet_size * n_chars]) noexcept
{
	if (word.size() > n_chars)
	{
		for (size_t i = 0; i < n_chars; i++)
			_unroll_char(word[word.size() - n_chars + i], v + alphabet_size * i);
	}
	else
	{
		for (size_t i = 0; i < n_chars - word.size(); i++)
			_unroll_char(' ', v + alphabet_size * i);
		for (size_t i = 0; i < word.size(); i++)
			_unroll_char(word[i], v + alphabet_size * (n_chars - word.size() + i));
	}
}

void mechan::Neuro::_unroll_message(const Parsed *message, float v[message_size]) noexcept
{
	if (message->words.size() > n_words)
	{
		for (size_t i = 0; i < n_words; i++)
			_unroll_word(message->words[message->words.size() - n_words + i].lowercase, v + alphabet_size * n_chars * i);
	}
	else
	{
		for (size_t i = 0; i < n_words - message->words.size(); i++)
			_unroll_word("", v + alphabet_size * n_chars * i);
		for (size_t i = 0; i < message->words.size(); i++)
			_unroll_word(message->words[i].lowercase, v + alphabet_size * n_chars * (n_words - message->words.size() + i));
	}
	v[alphabet_size * n_words * n_chars] = (message->end == '.') ? 1.0f : 0.0f;
	v[alphabet_size * n_words * n_chars + 1] = (message->end == '!') ? 1.0f : 0.0f;
	v[alphabet_size * n_words * n_chars + 2] = (message->end == '?') ? 1.0f : 0.0f;
}

bool mechan::Neuro::_train() noexcept
{
	if (_state == State::begin_training)
	{
		printf("Training...\n");
		_currect_message = 0;
		_currect_progress = 0;
		_state = State::training;
	}

	GLint sample = 0;
	while (sample < batch_size && _currect_message < (unsigned int)(train_part * _dialog->count()))
	{
		//Picking message pair
		unsigned int progress = (unsigned int)(10.0f * _currect_message / train_part / _dialog->count());
		if (progress > _currect_progress)
		{
			printf("%u0%% ", progress);
			_currect_progress = progress;
		}
		_currect_message++;
		std::string question, answer;
		if (!_dialog->message(_currect_message - 1, &question)) continue;
		if (!_dialog->message(_currect_message, &answer)) continue;

		//Positive sample
		Parsed parsed;
		parse_punctuation(question, &parsed);
		_unroll_message(&parsed, _input_buffer.data());
		parse_punctuation(answer, &parsed);
		_unroll_message(&parsed, _input_buffer.data() + message_size);
		_neuro.input()->store_row(sample, _input_buffer.data());
		_output_buffer[sample] = 1.0f;
		sample++;

		//Negative sample
		unsigned int random;
		std::string random_answer;
		while (true)
		{
			random = _distribution(_generator);
			if (_dialog->message(random, &random_answer)) break;
		}
		parse_punctuation(random_answer, &parsed);
		_unroll_message(&parsed, _input_buffer.data() + message_size);
		_neuro.input()->store_row(sample, _input_buffer.data());
		_output_buffer[sample] = 0.0f;
		sample++;
	}
	_neuro.goal()->store(_output_buffer.data());
	_neuro.batch_size() = sample;
	_neuro.forward();
	_neuro.backward();
	_neuro.correct();

	if (_currect_message >= (unsigned int)(train_part * _dialog->count()))
	{
		printf("\n");	
		_state = State::begin_testing;
	}
	return true;
}

bool mechan::Neuro::_test() noexcept
{
	if (_state == State::begin_testing)
	{
		printf("Testing...\n");
		_currect_message = 0;
		_currect_progress = 0;
		_train_pair_count = 0;
		_train_cost_count = 0.0f;
		_train_guess_count = 0;
		_test_pair_count = 0;
		_test_cost_count = 0.0f;
		_test_guess_count = 0;
		_state = State::train_testing;
	}

	unsigned int last_message;
	if ( _state == State::train_testing) last_message = (unsigned int)(train_part * _dialog->count());
	else last_message = (unsigned int)((train_part + test_part) * _dialog->count());
	
	GLint sample = 0;
	while (sample < batch_size && _currect_message < last_message)
	{
		//Picking message pair
		unsigned int progress = (unsigned int)(10.0f * _currect_message / (train_part + test_part) / _dialog->count());
		if (progress > _currect_progress)
		{
			printf("%u0%% ", progress);
			_currect_progress = progress;
		}
		_currect_message++;
		std::string question, answer;
		if (!_dialog->message(_currect_message - 1, &question)) continue;
		if (!_dialog->message(_currect_message, &answer)) continue;

		//Positive sample
		Parsed parsed;
		parse_punctuation(question, &parsed);
		_unroll_message(&parsed, _input_buffer.data());
		parse_punctuation(answer, &parsed);
		_unroll_message(&parsed, _input_buffer.data() + message_size);
		_neuro.input()->store_row(sample, _input_buffer.data());
		sample++;

		//Negative sample
		unsigned int random;
		std::string random_answer;
		while (true)
		{
			random = _distribution(_generator);
			if (_dialog->message(random, &random_answer)) break;
		}
		parse_punctuation(random_answer, &parsed);
		_unroll_message(&parsed, _input_buffer.data() + message_size);
		_neuro.input()->store_row(sample, _input_buffer.data());
		sample++;
	}
	_neuro.batch_size() = sample;
	_neuro.forward();
	_neuro.output()->load(_output_buffer.data());

	size_t back_sample = 0;
	while (back_sample < sample)
	{
		//Couning statistics
		float cost = (_output_buffer[back_sample] - 1.0f) * (_output_buffer[back_sample] - 1.0f);
		cost += ((_output_buffer[back_sample + 1] - 0.0f) * (_output_buffer[back_sample + 1] - 0.0f));
		if (_state == State::train_testing)
		{
			_train_pair_count++;
			if (_output_buffer[back_sample] > _output_buffer[back_sample + 1]) _train_guess_count++;
			_train_cost_count += cost;
		}
		else
		{
			_test_pair_count++;
			if (_output_buffer[back_sample] > _output_buffer[back_sample + 1]) _test_guess_count++;
			_test_cost_count += cost;
		}
		back_sample += 2;
	}

	if (_state == State::train_testing && _currect_message >= (unsigned int)(train_part * _dialog->count()))
	{
		_state = State::test_testing;
	}
	else if (_state == State::test_testing && _currect_message >= (unsigned int)((train_part + test_part) * _dialog->count()))
	{
		printf("\n");
		printf("Train samples: %u\n", 2 * _train_pair_count);
		printf("Train pairs:   %u\n", _train_pair_count);
		printf("Avarage cost:  %f\n", _train_cost_count / (2 * _train_pair_count));
		printf("Guess rate:    %f%%\n", 100.f * _train_guess_count / _train_pair_count);
		printf("Train samples: %u\n", 2 * _test_pair_count);
		printf("Train pairs:   %u\n", _test_pair_count);
		printf("Avarage cost:  %f\n", _test_cost_count / (2 * _test_pair_count));
		printf("Guess rate:    %f%%\n", 100.f * _test_guess_count / _test_pair_count);
		printf("\n");
		_state = State::begin_training;
	}
	return true;
}

mechan::Neuro::Neuro(Dialog *dialog, Word *word, bool train) noexcept :
	_dialog(dialog),
	_word(word),
	_distribution(0, dialog->count() - 1)
{
	if (!_input_buffer.resize(2 * message_size)) return;
	if (!_output_buffer.resize(batch_size)) return;
	if (!mathg::Default::init()) return;
	if (!mathg::MathG::init()) return;
	neurog::FullLayer::Info layer1(2 * message_size, 500, deviance);
	neurog::FullLayer::Info layer2(500, 1, deviance);
	neurog::Layer::Info *layers[] = { &layer1, &layer2 };
	if (_neuro.init("data/neuro", batch_size, train)) printf("Neuronal network found\n");
	else if (_neuro.init(2, layers, batch_size, train)) printf("Neuronal network created\n");
	else return;
	set_coefficient(coefficient);
	_generator.seed((unsigned int)time(nullptr));
	_last_save = clock();
	_currect_message = (unsigned int)(train_part * _dialog->count());
}

bool mechan::Neuro::ok() const noexcept
{
	return _neuro.ok();
}

float mechan::Neuro::get_coefficient() const noexcept
{
	return _neuro.coefficients(0);
}

void mechan::Neuro::set_coefficient(float coefficient) noexcept
{
	for (size_t i = 0; i < 2; i++) _neuro.coefficients(i) = coefficient;
}

bool mechan::Neuro::save() noexcept
{
	if (!_neuro.save("data/neuro")) return false;
	_last_save = clock();
	return true;
}

bool mechan::Neuro::train() noexcept
{
	if (_state == State::begin_training || _state == State::training)
	{
		if (!_train()) return false;
		if (clock() - _last_save > interval * CLOCKS_PER_SEC)
		{
			_neuro.save("data/neuro");
			_last_save = clock();
		}
	}
	else return _test();
	return true;
}

bool mechan::Neuro::store_messages(GLint sample, const Parsed *question, const Parsed *answer) noexcept
{
	if (question != nullptr) _unroll_message(question, _input_buffer.data());
	if (answer != nullptr) _unroll_message(answer, _input_buffer.data() + message_size);
	return _neuro.input()->store_row(sample, _input_buffer.data());
}

bool mechan::Neuro::load_heuristics(GLint batch_size, float ** const data) noexcept
{
	_neuro.batch_size() = batch_size;
	return _neuro.forward() && _neuro.output()->load(data);
}

mechan::Neuro::~Neuro() noexcept
{
	if (_neuro.ok()) _neuro.save("data/neuro");
	_neuro.finalize();
	mathg::MathG::finalize();
	mathg::Default::finalize();
}