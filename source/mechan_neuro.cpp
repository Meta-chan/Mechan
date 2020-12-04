#include "../header/mechan_neuro.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_dialog.h"
#include "../header/mechan_lowercase.h"
#include "../header/mechan.h"
#include <time.h>
#include <assert.h>

void mechan::Neuro::_unroll_char(unsigned char c, double v[33]) noexcept
{
	unsigned int nchar = (c >= 0xE0) ? (c - 0xE0) : 32;
	for (unsigned int i = 0; i < nchar; i++) v[i] = -1.0;
	v[nchar] = 1.0;
	for (unsigned int i = nchar + 1; i < 33; i++) v[i] = -1.0;
}

void mechan::Neuro::_unroll_message(const std::string message, double v[message_size]) noexcept
{
	unsigned int nwords = 0;
	unsigned int i = (unsigned int)message.size() - 1;
	while (true)
	{
		//Skip dots and spaces
		while (i != (unsigned int)-1)
		{
			char c = lowercase(message[i]);
			if (c >= 0xE0) break;
			else i--;
		}

		//Unroll word
		unsigned int nchars = 0;
		while (i != (unsigned int)-1)
		{
			char c = lowercase(message[i]);
			if (c >= 0xE0)
			{
				_unroll_char(c, v + 33 * nchars);
				nchars++;
				i--;
				if (nchars == nchars) break;
			}
			else if (c == '-')
			{
				i--;
			}
			else break;
		}

		//Skip charscters
		while (i != (unsigned int)-1)
		{
			char c = lowercase(message[i]);
			if (c >= 0xE0 || c == '-') i--;
			else break;
		}

		//Fill rest of input
		while (nchars < n_chars)
		{
			_unroll_char(' ', v + 33 * nchars);
			nchars++;
		}

		nwords++;
		if (nwords == n_words) break;
	}

	v[33 * n_words * n_chars] = (message.back() == '.') ? 1.0 : -1.0;
	v[33 * n_words * n_chars + 2] = (message.back() == '!') ? 1.0 : -1.0;
	v[33 * n_words * n_chars + 1] = (message.back() == '?') ? 1.0 : -1.0;
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

void mechan::Neuro::train() noexcept
{
	//unrolling message
	unsigned int nmessage = (*_distribution)(_generator);
	std::string question = mechan->dialog()->dialog(nmessage);
	if (question.empty()) return;
	std::string answer = mechan->dialog()->dialog(nmessage);
	if (answer.empty()) return;
	_unroll_message(question, _neuro->get_input()->data());

	if (((*_distribution)(_generator) % (1 + negative_pro_positive)) == 0)
	{
		//positive training
		_unroll_message(answer, _neuro->get_input()->data() + message_size);
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
			_unroll_message(random, _neuro->get_input()->data() + message_size);
			_neuro->forward();
			_neuro->get_goal()->at(0) = -1.0;
			_neuro->backward();
		}
	}
}

double mechan::Neuro::qestion_answer(const std::string question, const std::string answer) noexcept
{
	_unroll_message(question, _neuro->get_input()->data());
	_unroll_message(answer, _neuro->get_input()->data() + message_size);
	_neuro->forward();
	return _neuro->get_output()->at(0);
}

mechan::Neuro::~Neuro() noexcept
{
	if (_neuro != nullptr && _neuro->ok()) _neuro->save(WIDE_MECHAN_DIR "data\\neuro");
	delete _neuro;
}