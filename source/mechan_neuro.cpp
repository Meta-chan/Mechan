#define IR_INCLUDE 'i'
#define MATHG_INCLUDE
#define NEUROG_INCLUDE
#include "../header/mechan_neuro.h"
#include "../header/mechan_parse.h"
#include "../header/mechan_character.h"
#include "../header/mechan_config.h"
#include <mathg/default.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

int mechan::Neuro::word_number = 7;
int mechan::Neuro::char_number = 7;
int mechan::Neuro::char_size = 33;
int mechan::Neuro::interval = 3600;
int mechan::Neuro::batch_size = 1024;
bool mechan::Neuro::batch_correct = true;
int mechan::Neuro::layer2_size = 500;
int mechan::Neuro::layer3_size = 0;
float mechan::Neuro::train_part = 0.7f;
float mechan::Neuro::test_part = 0.3f;
float mechan::Neuro::coefficient = 0.001f;
float mechan::Neuro::deviance = 0.1f;

int mechan::Neuro::word_size;
int mechan::Neuro::message_size;

void mechan::Neuro::_unroll_char(char c, float *v) noexcept
{
	size_t nchar = is_cyrylic(c) ? ((unsigned char)c - (unsigned char)0xE0) : 32;
	for (size_t i = 0; i < nchar; i++) v[i] = 0.0f;
	v[nchar] = 1.0f;
	for (size_t i = nchar + 1; i < char_size; i++) v[i] = 0.0f;
}

void mechan::Neuro::_unroll_word(const std::string lowercase_word, float *v) noexcept
{
	if (lowercase_word == "")
	{
		for (size_t i = 0; i < char_number; i++) _unroll_char(' ', v + char_size * i);
		return;
	}

	//Unrolling letters
	if (lowercase_word.size() > char_number)
	{
		for (size_t i = 0; i < char_number; i++)
			_unroll_char(lowercase_word[lowercase_word.size() - char_number + i], v + char_size * i);
	}
	else
	{
		for (size_t i = 0; i < char_number - lowercase_word.size(); i++)
			_unroll_char(' ', v + char_size * i);
		for (size_t i = 0; i < lowercase_word.size(); i++)
			_unroll_char(lowercase_word[i], v + char_size * (char_number - lowercase_word.size() + i));
	}

	//Unrolling morphology characteristics
	Word::WordInfo const * info;
	unsigned int info_size;
	if (_word->word_info(lowercase_word, &info, &info_size))
	{
		for (size_t i = 0; i < MorphologyCharacteristics::number; i++)
		{
			v[char_number  * char_size + i] = info->probable_characteristics.get((MorphologyCharacteristic)i) ? 1.0f : 0.0f;
		}
	}
	else
	{
		for (size_t i = 0; i < MorphologyCharacteristics::number; i++)
		{
			v[char_number  * char_size + i] = 0.0f;
		}
	}

}

void mechan::Neuro::_unroll_message(const Parsed *message, float *v) noexcept
{
	if (message->words.size() > word_number)
	{
		for (size_t i = 0; i < word_number; i++)
			_unroll_word(message->words[message->words.size() - word_number + i].lowercase, v + word_size * i);
	}
	else
	{
		for (size_t i = 0; i < word_number - message->words.size(); i++)
			_unroll_word("", v + word_size * i);
		for (size_t i = 0; i < message->words.size(); i++)
			_unroll_word(message->words[i].lowercase, v + word_size * (word_number - message->words.size() + i));
	}
	v[char_size * word_number * char_number] = (message->end == '.') ? 1.0f : 0.0f;
	v[char_size * word_number * char_number + 1] = (message->end == '!') ? 1.0f : 0.0f;
	v[char_size * word_number * char_number + 2] = (message->end == '?') ? 1.0f : 0.0f;
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
	if (batch_correct) _neuro.correct();

	if (_currect_message >= (unsigned int)(train_part * _dialog->count()))
	{
		printf("\n");
		if (!batch_correct) _neuro.correct();
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
		float cost = _neuro.cost(_output_buffer[back_sample], 1.0f) + _neuro.cost(_output_buffer[back_sample + 1], 0.0f);
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
	if (strtol(Config::value("NEURO_WORD_NUMBER").c_str(), nullptr, 10) > 0)
		word_number = strtol(Config::value("NEURO_WORD_NUMBER").c_str(), nullptr, 10);
	if (strtol(Config::value("NEURO_CHAR_NUMBER").c_str(), nullptr, 10) > 0)
		char_number = strtol(Config::value("NEURO_CHAR_NUMBER").c_str(), nullptr, 10);
	if (strtol(Config::value("NEURO_INTERVAL").c_str(), nullptr, 10) > 0)
		interval = strtol(Config::value("NEURO_INTERVAL").c_str(), nullptr, 10);
	if (strtol(Config::value("NEURO_BATCH_SIZE").c_str(), nullptr, 10) > 0)
		batch_size = strtol(Config::value("NEURO_BATCH_SIZE").c_str(), nullptr, 10);
	if (Config::value("NEURO_BATCH_CORRECT") == "true" || Config::value("NEURO_BATCH_CORRECT") == "false")
		batch_correct = Config::value("NEURO_BATCH_CORRECT") == "true";
	if (strtol(Config::value("NEURO_LAYER2").c_str(), nullptr, 10) > 0)
		layer2_size = strtol(Config::value("NEURO_LAYER2").c_str(), nullptr, 10);
	if (strtol(Config::value("NEURO_LAYER3").c_str(), nullptr, 10) >= 0)
		layer3_size = strtol(Config::value("NEURO_LAYER3").c_str(), nullptr, 10);
	if (strtof(Config::value("NEURO_TRAIN_PART").c_str(), nullptr) > 0.0f)
		train_part = strtof(Config::value("NEURO_TRAIN_PART").c_str(), nullptr);
	if (strtof(Config::value("NEURO_TEST_PART").c_str(), nullptr) > 0.0f)
		test_part = strtof(Config::value("NEURO_TEST_PART").c_str(), nullptr);
	if (strtof(Config::value("NEURO_COEFFICIENT").c_str(), nullptr) > 0.0f)
		coefficient = strtof(Config::value("NEURO_COEFFICIENT").c_str(), nullptr);
	if (strtof(Config::value("NEURO_DEVIANCE").c_str(), nullptr) > 0.0f)
		deviance = strtof(Config::value("NEURO_DEVIANCE").c_str(), nullptr);
	word_size = char_size * char_number + MorphologyCharacteristics::number;
	message_size = word_size * word_number + 3;

	if (!_input_buffer.resize(2 * message_size)) return;
	if (!_output_buffer.resize(batch_size)) return;
	if (!mathg::Default::init()) return;
	if (!mathg::MathG::init()) return;
	neurog::FullLayer::Info layer1(2 * message_size, layer2_size, deviance, coefficient);
	neurog::FullLayer::Info layer2(layer2_size, layer3_size == 0 ? 1 : layer3_size, deviance, coefficient);
	if (layer3_size == 0)
	{
		neurog::Layer::Info *layers[] = { &layer1, &layer2 };
		if (_neuro.init("data/neuro", neurog::Cost::cross_entrophy, batch_size, train)) printf("Neuronal network found\n");
		else if (_neuro.init(2, layers, neurog::Cost::cross_entrophy, batch_size, train)) printf("Neuronal network created\n");
		else return;
	}
	else
	{
		neurog::FullLayer::Info layer3(layer3_size, 1, deviance, coefficient);
		neurog::Layer::Info *layers[] = { &layer1, &layer2, &layer3 };
		if (_neuro.init("data/neuro", neurog::Cost::cross_entrophy, batch_size, train)) printf("Neuronal network found\n");
		else if (_neuro.init(2, layers, neurog::Cost::cross_entrophy, batch_size, train)) printf("Neuronal network created\n");
		else return;
	}
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