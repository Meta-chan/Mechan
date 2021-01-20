#include "../header/mechan.h"
#include "../header/mechan_parse.h"
#include "../header/mechan_character.h"
#include <time.h>
#include <assert.h>

void mechan::Neuro::_unroll_char(char c, double v[33]) noexcept
{
	unsigned int nchar = is_cyrylic(c) ? ((unsigned char)c - (unsigned char)0xE0) : 32;
	for (unsigned int i = 0; i < nchar; i++) v[i] = -1.0;
	v[nchar] = 1.0;
	for (unsigned int i = nchar + 1; i < 33; i++) v[i] = -1.0;
}

void mechan::Neuro::_unroll_word(const std::string word, double v[33 * n_chars]) noexcept
{
	if (word.size() > n_chars)
	{
		for (unsigned int i = 0; i < n_chars; i++)
			_unroll_char(word[word.size() - n_chars + i], v + 33 * i);
	}
	else
	{
		for (unsigned int i = 0; i < n_chars - word.size(); i++)
			_unroll_char(' ', v + 33 * i);
		for (unsigned int i = 0; i < word.size(); i++)
			_unroll_char(word[i], v + 33 * (n_chars - word.size() + i));
	}
}

void mechan::Neuro::_unroll_message(const Parsed *message, double v[message_size]) noexcept
{
	if (message->words.size() > n_words)
	{
		for (unsigned int i = 0; i < n_words; i++)
			_unroll_word(message->words[message->words.size() - n_words + i].lowercase, v + 33 * n_chars * i);
	}
	else
	{
		for (unsigned int i = 0; i < n_words - message->words.size(); i++)
			_unroll_word("", v + 33 * n_chars * i);
		for (unsigned int i = 0; i < message->words.size(); i++)
			_unroll_word(message->words[i].lowercase, v + 33 * n_chars * (n_words - message->words.size() + i));
	}
	v[33 * n_words * n_chars] = (message->end == '.') ? 1.0 : -1.0;
	v[33 * n_words * n_chars + 1] = (message->end == '!') ? 1.0 : -1.0;
	v[33 * n_words * n_chars + 2] = (message->end == '?') ? 1.0 : -1.0;
}

mechan::Neuro::Neuro(Mechan *mechan) noexcept :
	_mechan(mechan),
	_distribution(0, mechan->dialog()->count() - 1)
{
	_generator.seed((unsigned int)time(nullptr));
	_neuro = new ir::Neuro<double>(SS("data\\neuro"), nullptr);
	if (!_neuro->ok())
	{
		delete _neuro;
		unsigned int layers[4] = { 2 * message_size, 2000, 2000, 1 };
		_neuro = new ir::Neuro<double>(4, layers, 0.01, nullptr);
	}
	if (_neuro->ok()) { _neuro->set_coefficient(0.01); _last_save = clock(); }
}

bool mechan::Neuro::ok() const noexcept
{
	return _neuro != nullptr && _neuro->ok();
}

double mechan::Neuro::get_coefficient() const noexcept
{
	return _neuro->get_coefficient();
}

void mechan::Neuro::set_coefficient(double coefficient) noexcept
{
	_neuro->set_coefficient(coefficient);
}

void mechan::Neuro::save() noexcept
{
	_neuro->save(SS("data\\neuro"));
	_last_save = clock();
}

void mechan::Neuro::train() noexcept
{
	//unrolling message
	unsigned int nmessage = _distribution(_generator);
	std::string question, answer;
	if (!_mechan->dialog()->message(nmessage, &question)) return;
	if (!_mechan->dialog()->message(nmessage + 1, &answer)) return;

	Parsed parsed;
	parse_punctuation(question, &parsed);
	_unroll_message(&parsed, _neuro->get_input()->data());

	if ((_distribution(_generator) % (1 + negative_pro_positive)) == 0)
	{
		//positive training
		parse_punctuation(answer, &parsed);
		_unroll_message(&parsed, _neuro->get_input()->data() + message_size);
		_neuro->forward();
		_neuro->get_goal()->at(0) = 1.0;
		_neuro->backward();

		char message[512];
		sprintf(message, "Positive training: %lf", _neuro->get_output()->at(0));
		_mechan->print_event_log(message);
	}
	else while (true)
	{
		//negative training
		nmessage = _distribution(_generator);
		std::string random;
		if (_mechan->dialog()->message(nmessage, &random))
		{
			parse_punctuation(random, &parsed);
			_unroll_message(&parsed, _neuro->get_input()->data() + message_size);
			_neuro->forward();
			_neuro->get_goal()->at(0) = -1.0;
			_neuro->backward();

			char message[512];
			sprintf(message, "Negative training: %lf", _neuro->get_output()->at(0));
			_mechan->print_event_log(message);
			break;
		}
	}

	//saving
	if (clock() - _last_save > 3600 * CLOCKS_PER_SEC)
	{
		_neuro->save(SS("data\\neuro"));
		_last_save = clock();
	}
}

double mechan::Neuro::qestion_answer(const Parsed *question, const Parsed *answer) noexcept
{
	_unroll_message(question, _neuro->get_input()->data());
	_unroll_message(answer, _neuro->get_input()->data() + message_size);
	_neuro->forward();
	return _neuro->get_output()->at(0);
}

mechan::Neuro::~Neuro() noexcept
{
	if (_neuro != nullptr && _neuro->ok()) _neuro->save(SS("data\\neuro"));
	delete _neuro;
}