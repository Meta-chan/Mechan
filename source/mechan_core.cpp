#include "../header/mechan_dialog.h"
#include "../header/mechan_core.h"
#include "../header/mechan_synonym.h"
#include "../header/mechan_neuro.h"
#include "../header/mechan.h"

#include "../header/mechan_parse.h"
#include <time.h>
#include <limits>

bool mechan::Core::_intersect(const std::vector<unsigned int> *a, const std::vector<unsigned int> *b) const noexcept
{
	for (unsigned int i = 0; i < a->size(); i++)
	{
		for (unsigned int j = 0; j < b->size(); j++)
		{
			if (a->at(i) == b->at(j)) return true;
		}
	}
	return false;
}

mechan::Core::Core() noexcept
	: _distribution(0, mechan->dialog()->count() - 1)
{
	_engine.seed((unsigned int)time(nullptr));
	_ok = true;
}

std::string mechan::Core::answer(Interface::Address address, const std::string question)  noexcept
{
	if (address.interface_id == Interface::ID::console && question == "shutdown")
	{
		_reqest_shutdown = true;
		return "Shutdown command recognized";
	}

	const unsigned int best_count = 100;
	struct
	{
		double heuristics = -std::numeric_limits<double>::infinity();
		std::string answer;
	} best[best_count];

	clock_t c = clock();
	while (clock() - c < 10 * CLOCKS_PER_SEC)
	{
		//Receiving question & answer pair
		unsigned int nmessage = _distribution(_engine);
		std::string question = mechan->dialog()->dialog(nmessage);
		std::string answer = mechan->dialog()->dialog(nmessage + 1);
		if (question.empty() || answer.empty()) continue;
		
		//Parsing question & answer
		std::vector<std::string> parsed_question;
		parse(question, &parsed_question);
		std::vector<std::string> parsed_answer;
		parse(answer, &parsed_answer);
		
		//Counting synonyms
		unsigned int synonym_count = 0;
		for (unsigned int i = 0; i < parsed_question.size(); i++)
		{
			std::vector<unsigned int> question_syngroups;
			mechan->synonym()->extended_syngroup(parsed_question[i], &question_syngroups);
			for (unsigned int j = 0; j < parsed_answer.size(); j++)
			{
				std::vector<unsigned int> answer_syngroups;
				mechan->synonym()->extended_syngroup(parsed_answer[j], &answer_syngroups);
				if (_intersect(&question_syngroups, &answer_syngroups)) synonym_count++;
			}
		}
		double heuritics = (double)synonym_count / (parsed_answer.size() * parsed_question.size());

		//Inserting in best table
		unsigned int place_in_best = (unsigned int)-1;
		for (unsigned int i = 0; i < best_count; i++)
		{
			if (heuritics > best[i].heuristics) place_in_best = i;
			else break;
		}
		if (place_in_best != (unsigned int)-1)
		{
			for (unsigned int i = 0; i < place_in_best; i++)
			{
				best[i].answer = best[i + 1].answer;
				best[i].heuristics = best[i + 1].heuristics;
			}
			best[place_in_best].answer = answer;
			best[place_in_best].heuristics = heuritics;
		}
	}

	//Passing throught neuronal net
	std::vector<std::string> parsed_question;
	parse(question, &parsed_question);
	for (unsigned int i = 0; i < best_count; i++)
	{
		std::vector<std::string> parsed_answer;
		parse(best[i].answer, &parsed_answer);
		best[i].heuristics = mechan->neuro()->qestion_answer(
			&parsed_question, question.back(),
			&parsed_answer, best[i].answer.back());
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

bool mechan::Core::request_shutdown() const noexcept
{
	return _reqest_shutdown;
}

bool mechan::Core::ok() const noexcept
{
	return _ok;
}