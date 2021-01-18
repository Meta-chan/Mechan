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
			//����� �������������� ��������������
			deprecated = 0,			//���������� �����
			reduced = 40,			//���������, ���� � �������������� � ���������

			//��� �����
			noun = 1,				//���������������
			adjective = 14,			//��������������
			conjunktion = 17,		//����
			interjection = 18,		//����������
			predicate = 19,			//���������� (�������, ������� ����� ��������� ����������)
			particle = 20,			//������� (���� ����� � ����������, �� ������ �� ����������)
			preposition = 22,		//�������
			adverb = 23,			//�������
			verb = 27,				//������
			participle = 38,		//��������� (��������������, ������� �������� �� ������� �������)
			transgressive = 41,		//������������ (�������, ���������� �� ������� �������)
			pronoun = 51,			//�����������
			introductory = 52,		//������� �����
			numeral = 57,			//���������

			//���������������. ���� � ���������������, ��������������, ���������. ����� ������������.
			inanimate = 2,
			animate = 15,

			//�����. ���� � ���������������, ������ ��������, ���������, ��������������
			singular = 3,
			plural = 13,

			//���. ���� � ���������������, ������ ��������, ���������, ��������������, ������������� ����������, �����������
			feminine = 4,
			maskuline = 6,
			neutrum = 16,
			common = 26,

			//�����. ���� � ���������������, ���������, ��������������, ������������, �����������
			nominative = 7,
			genetive = 8,
			dative = 9,
			accusative = 10,
			instrumnetal = 11,
			prepositional = 12,
			partitive = 53,
			counting = 54,
			vocative = 55,

			//������������. ���� � ��������, ���������, ������������
			transitive = 29,
			intransitive = 44,
			context = 49,

			//�����. ���� � ��������, ���������, ������������
			past = 31,
			present = 32,
			future = 36,

			//����. ���� � ��������
			first = 34,
			second = 35,
			third = 33,

			//�������������. ���� � ��������, ���������, ������������
			perfect = 42,
			imperfect = 43,

			//��� �������. ���� � �������
			definitive = 24,
			circumstantial = 59,
			itterogative = 66,

			//��� ���������������� �������. ���� � ��������������� �������
			qualitative = 25,
			grade = 46,
			method = 50,

			//��� ������������������� �������. ���� � ������������������ �������
			time = 60,
			place = 61,
			direction = 62,
			quantity = 63,
			reason = 65,
			goal = 67,

			//������������� �������. ���� � ��������������, ���������, �������, ������������
			comparative = 47,
			superlative = 48,

			//��� �������������. ���� � ������������
			ordinal = 58,
			collective = 64,
			indefinite = 68,

			//�������������� �������������� ���������������. ���� � ���������������
			indeclinable = 5,		//������������

			//�������������� �������������� ��������������. ���� � ��������������
			unchangable = 21,		//������������

			//�������������� �������������� ��������. ���� � ��������, ���������, ������������
			second_conjugation = 28,//������ ��� ���������, ���� � ��������, ���������, ������������
			imperative = 37,		//������������� �����
			reflexive = 45,			//�������������� �� ����, � ��������, ���������, ������������
			infinitive = 30,		//���������, � ��������

			//�������������� �������������� ���������. ���� � ���������
			passive = 39			//������������� �����, � ���������
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