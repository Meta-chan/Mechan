#define IR_INCLUDE 'i'
#include "../header/mechan_core.h"
#include "../header/mechan_parse.h"
#include <time.h>
#include <limits>
#include <algorithm>

bool mechan::Core::_intersect(const std::vector<size_t> *a, const unsigned int *b, size_t bsize) const noexcept
{
	size_t ia = 0;
	size_t ib = 0;
	while (true)
	{
		if (ia == a->size() || ib == bsize) return false;
		else if (a->at(ia) == b[ib]) return true;
		else if (a->at(ia) < b[ib]) ia++;
		else ib++;
	}
	return false;
}

mechan::Core::Core(Dialog *dialog, Word *word) noexcept :
	_dialog(dialog),
	_word(word)
{
	_ok = true;
}

std::string mechan::Core::answer(const std::string question)  noexcept
{
	//Parsing question
	std::vector<std::vector<size_t>> question_syngroups; 
	{
		Parsed parsed_question;
		parse_punctuation(question, &parsed_question);
		question_syngroups.resize(parsed_question.words.size());
		for (size_t i = 0; i < parsed_question.words.size(); i++)
		{
			const Word::WordInfo *info;
			unsigned int size;
			if (_word->word_info(parsed_question.words[i].lowercase, &info, &size))
			{
				question_syngroups[i].resize(info->synonym_group_number(size));
				for (size_t j = 0; j < question_syngroups[i].size(); j++)
					question_syngroups[i][j] = (info->synonym_groups())[j];
			}
			else parsed_question.words[i].lowercase.resize(0);
		}
	}

	//Creating table of best
	struct Best
	{
		std::string answer;
		float heuristics;
	};
	std::vector<Best> best;
	try { best.resize(best_size); } catch (...) { return std::string(); }

	//Processing random messages
	std::uniform_int_distribution<unsigned int> distribution(0, _dialog->count() - 1);
	std::default_random_engine engine;
	engine.seed((unsigned int)time(nullptr));
	clock_t c = clock();

	while (clock() - c < timeout * CLOCKS_PER_SEC)
	{
		//Receiving question & answer pair
		unsigned int random_question_number = distribution(engine);
		std::string random_question, random_answer;
		if (!_dialog->message(random_question_number, &random_question)) continue;
		if (!_dialog->message(random_question_number + 1, &random_answer)) continue;

		//Parsing random question
		Parsed parsed_random_question;
		parse_punctuation(random_question, &parsed_random_question);

		//Counting synonyms
		size_t synonym_count = 0;
		for (size_t i = 0; i < question_syngroups.size(); i++)
		{
			for (size_t j = 0; j < parsed_random_question.words.size(); j++)
			{
				const Word::WordInfo *random_question_info;
				unsigned int size;
				if (_word->word_info(parsed_random_question.words[j].lowercase, &random_question_info, &size)
				&& _intersect(&question_syngroups[i], random_question_info->synonym_groups(), random_question_info->synonym_group_number(size)))
					synonym_count++;
			}
		}
		float heuritics = (float)synonym_count / (question_syngroups.size() * parsed_random_question.words.size());

		//Inserting in best table
		size_t place_in_best = (size_t)-1;
		for (size_t i = 0; i < best_size; i++)
		{
			if (heuritics > best[i].heuristics) place_in_best = i;
			else break;
		}
		if (place_in_best != (size_t)-1)
		{
			for (size_t i = 0; i < place_in_best; i++)
			{
				best[i].answer = best[i + 1].answer;
				best[i].heuristics = best[i + 1].heuristics;
			}
			best[place_in_best].answer = random_answer;
			best[place_in_best].heuristics = heuritics;
		}
	}

	//Mixing outputs of threads
	std::sort(&best.front(), &best.back() + 1, [](const Best &a, const Best &b) { return a.heuristics < b.heuristics; });

	//Passing throught neuronal net
	Parsed parsed_question;
	parse_punctuation(question, &parsed_question);
	_neuro->store_messages(0, &parsed_question, nullptr);
	for (GLint total_sample = 0; total_sample < best_size; total_sample += Neuro::batch_size)
	{
		Parsed parsed_answer;
		GLint sample;
		for (sample = 0; sample < Neuro::batch_size && total_sample + sample < best_size; sample++)
		{
			parse_punctuation(best[total_sample + sample].answer, &parsed_question);
			_neuro->store_messages(sample, nullptr, &parsed_answer);
		}

		float *neuro_heuristics;
		_neuro->load_heuristics(sample, &neuro_heuristics);
		for (sample = 0; sample < Neuro::batch_size && total_sample + sample < best_size; sample++)
		{
			best[total_sample + sample].heuristics = neuro_heuristics[sample];
		}
	}

	//Searching for maximum
	float best_heuristic = -std::numeric_limits<float>::infinity();
	std::string best_answer;
	for (GLint i = 0; i < best_size; i++)
	{
		if (best[i].heuristics > best_heuristic)
		{
			best_heuristic = best[i].heuristics;
			best_answer = best[i].answer;
		}
	}

	return best_answer;
}

bool mechan::Core::ok() const noexcept
{
	return _ok;
}
