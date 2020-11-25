#pragma once

#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>
#include <vector>

namespace mechan
{
	class Morphology
	{
	private:
		struct OffsetGroupItem
		{
			unsigned int lowercase_word;
			unsigned int spelling;
			unsigned int characteristics;
		};

		class Parser
		{
		private:
			ir::S2STDatabase *_word2group	= nullptr;
			ir::N2STDatabase *_group2data	= nullptr;
			FILE *_morphology				= nullptr; 
			
			std::vector<std::string> _characteristics;
			std::vector<char> _continuous_buffer;
			std::vector<char> _string_buffer;
			std::string _characteristics_buffer;
			std::vector<OffsetGroupItem> _item_buffer;

			unsigned int _group = 0;
			bool _deprecated;

			void _skip_space();
			void _skip_space_delimiter();
			void _skip_space_asterisk();
			void _read_word(bool spelling);
			void _read_characteristics();
			void _add_characteristic();
			bool _skip_line();

		public:
			bool parse();
			~Parser();
		};

	public:
		enum class Characteristic
		{
			//����� �������������� ��������������
			deprecated = 255,	//���������� �����
			reduced = 40,	//���������, ���� � �������������� � ���������

			//��� �����
			noun = 1,	//���������������
			adjective = 14,	//��������������
			conjunktion = 17,	//����
			interjection = 18,	//����������
			predicate = 19,	//���������� (�������, ������� ����� ��������� ����������)
			particle = 20,	//������� (���� ����� � ����������, �� ������ �� ����������)
			preposition = 22,	//�������
			adverb = 23,	//�������
			verb = 27,	//������
			participle = 38,	//��������� (��������������, ������� �������� �� ������� �������)
			transgressive = 41,	//������������ (�������, ���������� �� ������� �������)
			pronoun = 51,	//�����������
			introductory = 52,	//������� �����
			numeral = 57,	//���������

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
			indeclinable = 5,	//������������

			//�������������� �������������� ��������������. ���� � ��������������
			unchangable = 21,	//������������

			//�������������� �������������� ��������. ���� � ��������, ���������, ������������
			second_conjugation = 28,	//������ ��� ���������, ���� � ��������, ���������, ������������
			imperative = 37,	//������������� �����
			reflexive = 45,	//�������������� �� ����, � ��������, ���������, ������������
			infinitive = 30,	//���������, � ��������

			//�������������� �������������� ���������. ���� � ���������
			passive = 39	//������������� �����, � ���������
		};

		struct GroupItem
		{
			const char *lowercase_word;
			const char *spelling;
			const Characteristic *characteristics;
		};

	private:
		bool _ok = false;
		ir::S2STDatabase *_word2group;
		ir::N2STDatabase *_group2data;

		
	public:
		Morphology();
		void word_info(const char *lowercase_word, std::vector<unsigned int> *groups);
		void group_info(unsigned int group, std::vector<GroupItem> *data);
		~Morphology();
	};
}