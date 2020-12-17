#pragma once

#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>
#include <vector>

namespace mechan
{
	class Mechan;

	class Morphology
	{
	public:
		enum class Characteristic
		{
			//Общие дополнительные характеристики
			deprecated = 255,	//Устаревшее слово
			reduced = 40,	//Краткость, есть у прилагательных и причастий

			//Тип слова
			noun = 1,	//существительное
			adjective = 14,	//прилагательное
			conjunktion = 17,	//союз
			interjection = 18,	//междометие
			predicate = 19,	//предикатив (наречие, которое может выступать подлежащим)
			particle = 20,	//частица (несёт смысл и содержание, не влияет на грамматику)
			preposition = 22,	//предлог
			adverb = 23,	//наречие
			verb = 27,	//глагол
			participle = 38,	//причастие (прилагательное, которое отвечает на вопросы глагола)
			transgressive = 41,	//деепричастие (наречие, отвечающее на вопросы глагола)
			pronoun = 51,	//местоимение
			introductory = 52,	//вводное слово
			numeral = 57,	//числитель

			//Воодушевлённость. Есть у существительных, прилагательных, причастий. Лучше игнорировать.
			inanimate = 2,
			animate = 15,

			//Число. Есть у существительных, иногда глаголов, причастий, прилагательных
			singular = 3,
			plural = 13,

			//Пол. Есть у существительных, иногда глаголов, причастий, прилагательных, собирательных числителей, местоимений
			feminine = 4,
			maskuline = 6,
			neutrum = 16,
			common = 26,

			//Падеж. Есть у существительных, причастий, прилагательных, числительных, местоимений
			nominative = 7,
			genetive = 8,
			dative = 9,
			accusative = 10,
			instrumnetal = 11,
			prepositional = 12,
			partitive = 53,
			counting = 54,
			vocative = 55,

			//Переходность. Есть у глаголов, причастий, деепричатсий
			transitive = 29,
			intransitive = 44,
			context = 49,

			//Время. Есть у глаголов, причастий, деепричастий
			past = 31,
			present = 32,
			future = 36,

			//Лицо. Есть у глаголов
			first = 34,
			second = 35,
			third = 33,

			//Совершённость. Есть у глаголов, причастий, деепричастий
			perfect = 42,
			imperfect = 43,

			//Тип наречия. Есть у наречий
			definitive = 24,
			circumstantial = 59,
			itterogative = 66,

			//Тип определительного наречия. Есть у определительных наречий
			qualitative = 25,
			grade = 46,
			method = 50,

			//Тип обстоятельственного наречия. Есть у обстоятельственных наречий
			time = 60,
			place = 61,
			direction = 62,
			quantity = 63,
			reason = 65,
			goal = 67,

			//Сравнительная степень. Есть у прилагательных, причастий, наречий, деепричастий
			comparative = 47,
			superlative = 48,

			//Тип числительного. Есть у числительных
			ordinal = 58,
			collective = 64,
			indefinite = 68,

			//Дополнительные характеристики существительных. Есть у существительных
			indeclinable = 5,	//Несклоняемое

			//Дополнительные характеристики прилагательных. Есть у прилагательных
			unchangable = 21,	//Неизменяемое

			//Дополнительные характеристики глаголов. Есть у глаголов, причастий, деепричастий
			second_conjugation = 28,	//Второй тип спряжения, есть у глаголов, причастий, деепричастий
			imperative = 37,	//Поселительный залог
			reflexive = 45,	//Направленность на себя, у глаголов, причастий, деепричастий
			infinitive = 30,	//Инфинитив, у глаголов

			//Дополнительные характеристики причастий. Есть у причастий
			passive = 39	//Страдательный залог, у причастий
		};

		struct OffsetGroupItem
		{
			unsigned int lowercase_word;
			unsigned int spelling;
			unsigned int characteristics;
		};

		struct GroupItem
		{
			const char *lowercase_word;
			const char *spelling;
			const Characteristic *characteristics;
		};

	private:
		Mechan *_mechan					= nullptr;
		ir::S2STDatabase *_word2group	= nullptr;
		ir::N2STDatabase *_group2data	= nullptr;
		
	public:
		Morphology(Mechan *mechan)															noexcept;
		bool ok()																			const noexcept;
		void word_info(const std::string lowercase_word, std::vector<unsigned int> *groups)	const noexcept;
		void group_info(unsigned int group, std::vector<GroupItem> *data)					const noexcept;
		~Morphology();
	};
}