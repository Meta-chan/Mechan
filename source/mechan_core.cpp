#define IR_INCLUDE 'i'
#include "../header/mechan.h"
#include "../header/mechan_parse.h"
#ifdef OMP_FOUND
	#include <omp.h>
#endif
#include <time.h>
#include <limits>
#include <algorithm>

bool mechan::Core::_intersect(const std::vector<unsigned int> *a, const unsigned int *b, unsigned int bsize) const noexcept
{
	unsigned int ia = 0;
	unsigned int ib = 0;
	while (true)
	{
		if (ia == a->size() || ib == bsize) return false;
		else if (a->at(ia) == b[ib]) return true;
		else if (a->at(ia) < b[ib]) ia++;
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
		Parsed parsed_question;
		parse_punctuation(question, &parsed_question);
		question_syngroups.resize(parsed_question.words.size());
		for (unsigned int i = 0; i < parsed_question.words.size(); i++)
		{
			const Word::WordInfo *info;
			unsigned int size;
			if (_mechan->word()->word_info(parsed_question.words[i].lowercase, &info, &size))
			{
				question_syngroups[i].resize(info->synonym_group_number(size));
				for (unsigned int j = 0; j < question_syngroups[i].size(); j++)
					question_syngroups[i][j] = (info->synonym_groups())[j];
			}
			else parsed_question.words[i].lowercase.resize(0);
		}
	}

	//Creating table of best
	struct Best
	{
		double heuristics = -std::numeric_limits<double>::infinity();
		std::string answer;
	};
	const unsigned int thread_best_size = 2;
	std::vector<Best> best;

	//Processing random messages
	#ifdef OMP_FOUND
	#pragma omp parallel shared(question_syngroups, best, thread_best_size)
	{
		#pragma omp single
		{
			best.resize(omp_get_num_threads() * thread_best_size);
		}
	#else
		best.resize(thread_best_size);
	#endif
		std::uniform_int_distribution<unsigned int> distribution(0, _mechan->dialog()->count() - 1);
		std::default_random_engine engine;
		#ifdef OMP_FOUND
			engine.seed((unsigned int)time(nullptr) + omp_get_thread_num());
		#else
			engine.seed((unsigned int)time(nullptr));
		#endif
		clock_t c = clock();

		while (clock() - c < 10 * CLOCKS_PER_SEC)
		{
			//Receiving question & answer pair
			unsigned int random_question_number = distribution(engine);
			std::string random_question, random_answer;
			if (!_mechan->dialog()->message(random_question_number, &random_question)) continue;
			if (!_mechan->dialog()->message(random_question_number + 1, &random_answer)) continue;

			//Parsing random question
			Parsed parsed_random_question;
			parse_punctuation(random_question, &parsed_random_question);

			//Counting synonyms
			unsigned int synonym_count = 0;
			for (unsigned int i = 0; i < question_syngroups.size(); i++)
			{
				for (unsigned int j = 0; j < parsed_random_question.words.size(); j++)
				{
					const Word::WordInfo *random_question_info;
					unsigned int size;
					if (_mechan->word()->word_info(parsed_random_question.words[j].lowercase, &random_question_info, &size)
					&& _intersect(&question_syngroups[i], random_question_info->synonym_groups(), random_question_info->synonym_group_number(size)))
						synonym_count++;
				}
			}
			double heuritics = (double)synonym_count / (question_syngroups.size() * parsed_random_question.words.size());

			//Inserting in best table
			unsigned int place_in_best = (unsigned int)-1;
			#ifdef OMP_FOUND
				unsigned int thread_first_best = thread_best_size * omp_get_thread_num();
			#else
				unsigned int thread_first_best = 0;
			#endif
			unsigned int thread_last_best = thread_first_best + thread_best_size;
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
	#ifdef OMP_FOUND
	}
	#endif

	//Mixing outputs of threads
	std::sort(best.begin(), best.end(), [](const Best &a, const Best &b) { return a.heuristics < b.heuristics; });

	//Passing throught neuronal net
	Parsed parsed_question;
	parse_punctuation(question, &parsed_question);
	for (unsigned int i = 0; i < thread_best_size; i++)
	{
		if (!best[i].answer.empty())
		{
			Parsed parsed_answer;
			parse_punctuation(best[i].answer, &parsed_answer);
			best[i].heuristics = _mechan->neuro()->qestion_answer(&parsed_question, &parsed_answer);
		}
	}

	//Choosing best of the best
	double best_heuristics = -std::numeric_limits<double>::infinity();
	std::string best_answer;
	for (unsigned int i = 0; i < thread_best_size; i++)
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
