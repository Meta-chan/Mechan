#include "../header/mechan_dialog.h"
#include "../header/mechan_neuro.h"
#include "../header/mechan.h"

#include "../header/mechan_parse.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_lowercase.h"
#include <time.h>
#include <assert.h>

void mechan::Neuro::_unroll_char(char c, double v[33]) noexcept
{
	unsigned int nchar = is_lowercase(c) ? (c - (unsigned char)0xE0) : 32;
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

void mechan::Neuro::_unroll_message(
	const std::vector<std::string> *message, char message_type,
	double v[message_size]) noexcept
{
	if (message->size() > n_words)
	{
		for (unsigned int i = 0; i < n_words; i++)
			_unroll_word(message->at(message->size() - n_words + i), v + 33 * n_chars * i);
	}
	else
	{
		for (unsigned int i = 0; i < n_words - message->size(); i++)
			_unroll_word("", v + 33 * n_chars * i);
		for (unsigned int i = 0; i < message->size(); i++)
			_unroll_word(message->at(i), v + 33 * n_chars * (n_words - message->size() + i));
	}
	v[33 * n_words * n_chars] = (message_type == '.') ? 1.0 : -1.0;
	v[33 * n_words * n_chars + 2] = (message_type == '!') ? 1.0 : -1.0;
	v[33 * n_words * n_chars + 1] = (message_type == '?') ? 1.0 : -1.0;
}

mechan::Neuro::Neuro() noexcept
{
	ir::ec code;
	_generator.seed((unsigned int)time(nullptr));
	_distribution = new std::uniform_int_distribution<unsigned int>(mechan->dialog()->count());
	_neuro = new ir::Neuro<double>(WIDE_MECHAN_DIR "\\data\\neuro", &code);
}

bool mechan::Neuro::ok() const noexcept
{
	return _neuro != nullptr && _neuro->ok();
}

void mechan::Neuro::save() const noexcept
{
	_neuro->save(WIDE_MECHAN_DIR);
}

void mechan::Neuro::train() noexcept
{
	//unrolling message
	unsigned int nmessage = (*_distribution)(_generator);
	std::string question = mechan->dialog()->dialog(nmessage);
	if (question.empty()) return;
	std::string answer = mechan->dialog()->dialog(nmessage);
	if (answer.empty()) return;

	std::vector<std::string> parsed;
	parse(question, &parsed);
	_unroll_message(&parsed, question.back(), _neuro->get_input()->data());

	if (((*_distribution)(_generator) % (1 + negative_pro_positive)) == 0)
	{
		//positive training
		parse(answer, &parsed);
		_unroll_message(&parsed, answer.back(), _neuro->get_input()->data() + message_size);
		_neuro->forward();
		_neuro->get_goal()->at(0) = 1.0;
		_neuro->backward();
	}
	else while (true)
	{
		//negative training
		nmessage = (*_distribution)(_generator);
		std::string random = mechan->dialog()->dialog(nmessage);
		if (!random.empty())
		{
			parse(random, &parsed);
			_unroll_message(&parsed, random.back(), _neuro->get_input()->data() + message_size);
			_neuro->forward();
			_neuro->get_goal()->at(0) = -1.0;
			_neuro->backward();
		}
	}
}

double mechan::Neuro::qestion_answer(
	const std::vector<std::string> *question, char question_type,
	const std::vector<std::string> *answer, char answer_type) noexcept
{
	_unroll_message(question, question_type, _neuro->get_input()->data());
	_unroll_message(answer, answer_type, _neuro->get_input()->data() + message_size);
	_neuro->forward();
	return _neuro->get_output()->at(0);
}

mechan::Neuro::~Neuro() noexcept
{
	if (_neuro != nullptr && _neuro->ok()) _neuro->save(WIDE_MECHAN_DIR "data\\neuro");
	delete _neuro;
}