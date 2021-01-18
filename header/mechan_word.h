#pragma once

#include <ir_database/ir_s2st_database.h>
#include <ir_database/ir_n2st_database.h>
#include <map>
#include <set>
#include <vector>
#include <stdio.h>

namespace mechan
{
	class Mechan;

	class Word
	{
	public:
		enum class MorphologyCharacteristic
		{
			//Общие дополнительные характеристики
			deprecated = 0,			//Устаревшее слово
			reduced = 40,			//Краткость, есть у прилагательных и причастий

			//Тип слова
			noun = 1,				//существительное
			adjective = 14,			//прилагательное
			conjunktion = 17,		//союз
			interjection = 18,		//междометие
			predicate = 19,			//предикатив (наречие, которое может выступать подлежащим)
			particle = 20,			//частица (несёт смысл и содержание, не влияет на грамматику)
			preposition = 22,		//предлог
			adverb = 23,			//наречие
			verb = 27,				//глагол
			participle = 38,		//причастие (прилагательное, которое отвечает на вопросы глагола)
			transgressive = 41,		//деепричастие (наречие, отвечающее на вопросы глагола)
			pronoun = 51,			//местоимение
			introductory = 52,		//вводное слово
			numeral = 57,			//числитель

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
			indeclinable = 5,		//Несклоняемое

			//Дополнительные характеристики прилагательных. Есть у прилагательных
			unchangable = 21,		//Неизменяемое

			//Дополнительные характеристики глаголов. Есть у глаголов, причастий, деепричастий
			second_conjugation = 28,//Второй тип спряжения, есть у глаголов, причастий, деепричастий
			imperative = 37,		//Поселительный залог
			reflexive = 45,			//Направленность на себя, у глаголов, причастий, деепричастий
			infinitive = 30,		//Инфинитив, у глаголов

			//Дополнительные характеристики причастий. Есть у причастий
			passive = 39			//Страдательный залог, у причастий
		};

		class MorphologyCharacteristics
		{
		private:
			unsigned int _a = 0, _b = 0, _c = 0;

		public:
			bool get(MorphologyCharacteristic c)										const noexcept;
			void set(MorphologyCharacteristic c, bool v)								noexcept;
		};

		struct WordInfo
		{
			unsigned int lowercase_occurence_number;
			unsigned int uppercase_occurence_number;
			MorphologyCharacteristics probable_characteristics;
			unsigned int morphology_group_number;
			unsigned int *morphology_groups()						noexcept;		//sorted
			const unsigned int *morphology_groups()					const noexcept;	//sorted
			unsigned int synonym_group_number(unsigned int size)	const noexcept;
			unsigned int *synonym_groups()							noexcept;		//sorted
			const unsigned int *synonym_groups()					const noexcept;	//sorted
		};

	private:

		struct UnpackedWordInfo
		{
			unsigned int lowercase_occurence_number = 0;
			unsigned int uppercase_occurence_number = 0;
			std::map<unsigned int, MorphologyCharacteristics> morphology_characteristics;	//sorted by group, only first characteristics for each group
			std::set<unsigned int> synonym_groups;											//sorted by group
		};

		struct UnpackedMorphologyGroupInfo
		{
			std::set<std::string> lowercase_words;	//not sorted
		};

		struct UnpackedSynonymGroupInfo
		{
			std::set<std::string> lowercase_words;	//not sorted
		};
	
		Mechan *_mechan							= nullptr;
		ir::S2STDatabase *_words				= nullptr;
		std::map<std::string, UnpackedWordInfo> _unpacked_words;
		std::vector<UnpackedMorphologyGroupInfo> _unpacked_morphology_groups;
		std::vector<UnpackedSynonymGroupInfo> _unpacked_sysnonym_groups;
		std::vector<char> _buffer;

		//Open existing databases
		bool _load()				noexcept;
		
		//Utilities
		void _parse_morphology_add(unsigned int group, const std::string lowercase_word, MorphologyCharacteristics ch) noexcept;
		void _parse_synonym_add(unsigned int group, const std::string lowercase_word) noexcept;
		unsigned int _pack_morphology_group_occurencies(unsigned int group) const noexcept;

		//Creating databases
		bool _parse_dialog()		noexcept;
		bool _parse_morphology()	noexcept;
		bool _parse_synonym()		noexcept;
		bool _pack()				noexcept;

	public:
		Word(Mechan *mechan)		noexcept;
		bool ok()					const noexcept;
		bool word_info(std::string lowercase_word, const WordInfo **info, unsigned int *info_size) const noexcept;
		~Word()						noexcept;
	};
}