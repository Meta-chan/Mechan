#include "../header/mechan.h"
#include "../header/mechan_parse.h"
#include <omp.h>
#include <time.h>
#include <limits>

bool mechan::Core::_intersect(const std::vector<unsigned int> *a, const std::vector<unsigned int> *b) const noexcept
{
	unsigned int ia = 0;
	unsigned int ib = 0;
	while (true)
	{
		if (ia == a->size() || ib == b->size()) return false;
		else if (a->at(ia) == b->at(ib)) return true;
		else if (a->at(ia) < b->at(ib)) ia++;
		else ib++;
	}
	return false;
}

mechan::Core::Core(Mechan *mechan) noexcept : _mechan(mechan)
{
	_ok = true;
}

std::string mechan::Core::answer(const std::string question)  noexcept
{
	//Parsing question
	std::vector<std::vector<unsigned int>> question_syngroups; 
	{
		std::vector<std::string> parsed_question;
		parse(question, &parsed_question, true);
		question_syngroups.resize(parsed_question.size());
		for (unsigned int i = 0; i < parsed_question.size(); i++)
			_mechan->synonym()->extended_syngroup(parsed_question[i], &question_syngroups[i]);
	}

	//Creating table of best
	const unsigned int best_count = 100;
	struct
	{
		double heuristics = -std::numeric_limits<double>::infinity();
		std::string answer;
	} best[best_count];

	//Processing random messages
	#pragma omp parallel shared(question_syngroups, best)
	{
		std::uniform_int_distribution<unsigned int> distribution(0, _mechan->dialog()->count() - 1);
		std::default_random_engine engine;
		engine.seed((unsigned int)time(nullptr) + omp_get_thread_num());
		clock_t c = clock();

		while (clock() - c < 10 * CLOCKS_PER_SEC)
		{
			//Receiving question & answer pair
			unsigned int random_question_number = distribution(engine);
			std::string random_question = _mechan->dialog()->dialog(random_question_number);
			std::string random_answer = _mechan->dialog()->dialog(random_question_number + 1);
			if (random_question.empty() || random_answer.empty()) continue;

			//Parsing random question
			std::vector<std::string> parsed_random_question;
			parse(random_question, &parsed_random_question, true);

			//Counting synonyms
			unsigned int synonym_count = 0;
			for (unsigned int i = 0; i < question_syngroups.size(); i++)
			{
				for (unsigned int j = 0; j < parsed_random_question.size(); j++)
				{
					std::vector<unsigned int> random_question_syngroups;
					_mechan->synonym()->extended_syngroup(parsed_random_question[j], &random_question_syngroups);
					if (_intersect(&question_syngroups[i], &random_question_syngroups)) synonym_count++;
				}
			}
			double heuritics = (double)synonym_count / (question_syngroups.size() * parsed_random_question.size());

			//Inserting in best table
			unsigned int place_in_best = (unsigned int)-1;
			unsigned int thread_first_best = best_count * omp_get_thread_num() / omp_get_num_threads();
			unsigned int thread_last_best = best_count * (omp_get_thread_num() + 1) / omp_get_num_threads();
			for (unsigned int i = thread_first_best; i < thread_last_best; i++)
			{
				if (heuritics > best[i].heuristics) place_in_best = i;
				else break;
			}
			if (place_in_best != (unsigned int)-1)
			{
				for (unsigned int i = thread_first_best; i < place_in_best; i++)
				{
					best[i].answer = best[i + 1].answer;
					best[i].heuristics = best[i + 1].heuristics;
				}
				best[place_in_best].answer = random_answer;
				best[place_in_best].heuristics = heuritics;
			}
		}
	}

	//Passing throught neuronal net
	std::vector<std::string> parsed_question;
	parse(question, &parsed_question, true);
	for (unsigned int i = 0; i < best_count; i++)
	{
		if (!best[i].answer.empty())
		{
			std::vector<std::string> parsed_answer;
			parse(best[i].answer, &parsed_answer, true);
			best[i].heuristics = _mechan->neuro()->qestion_answer(
				&parsed_question, question.back(),
				&parsed_answer, best[i].answer.back());
		}
	}

	//Choosing best of the best
	double best_heuristics = -std::numeric_limits<double>::infinity();
	std::string best_answer;
	for (unsigned int i = 0; i < best_count; i++)
	{
		if (best[i].heuristics > best_heuristics)
		{
			best_heuristics = best[i].heuristics;
			best_answer = best[i].answer;
		}
	}

	return best_answer;
}

bool mechan::Core::ok() const noexcept
{
	return _ok;
}