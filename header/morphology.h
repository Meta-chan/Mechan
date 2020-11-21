#ifndef MORPHOLOGY
#define MORPHOLOGY

#define M_DEPRECATED 255

enum M_TYPE
{
	noun = 1,			//���������������
	adjective = 14,		//��������������
	conjunktion = 17,	//����
	interjection = 18,	//����������
	predicate = 19,		//���������� (�������, ������� ����� ��������� ����������)
	particle = 20,		//������� (���� ����� � ����������, �� ������ �� ����������)
	preposition = 22,	//�������
	adverb = 23,		//�������
	verb = 27,			//������
	participle = 38,	//��������� (��������������, ������� �������� �� ������� �������)
	transgressive = 41,	//������������ (�������, ���������� �� ������� �������)
	pronoun = 51,		//�����������
	introductory = 52,	//������� �����
	numeral = 57		//���������
};

enum M_ANIMATE			//���� � ���������������, ��������������, ���������. ����� ������������.
{
	inanimate = 2,
	animate = 15
};

enum M_NUMBER			//���� � ���������������, ������ ��������, ���������, ��������������
{
	singular = 3,
	plural = 13
};

enum M_GENDER			//���� � ���������������, ������ ��������, ���������, ��������������, ������������� ����������, �����������
{
	feminine = 4,
	maskuline = 6,
	neutrum = 16,
	common = 26
};

enum M_CASE				//���� � ���������������, ���������, ��������������, ������������, �����������
{
	nominative = 7,
	genetive = 8,
	dative = 9,
	accusative = 10,
	instrumnetal = 11,
	prepositional = 12,
	partitive = 53,
	counting = 54,
	vocative = 55
};

#define M_INDECLINABLE_NOUN	5		//������������, ������ � ���������������
#define M_UNCHANGABLE_ADJECTIVE 21	//������������, ������ � ��������������

#define M_VERB_SECOND_CONJUGATION 28//������ ��� ���������, � ��������, ���������, ������������

enum M_TRANSITIVITY					//������������, � ��������, ���������, ������������
{
	transitive = 29,
	intransitive = 44,
	context = 49
};

#define M_VERB_INFINITIVE 30		//���������, � ��������

enum M_TENSE						//�����, ���� � ��������, ���������, ������������
{
	past = 31,
	present = 32,
	future = 36,
};

enum M_PERSON						//����, � ��������
{
	first = 34,
	second = 35,
	third = 33
};

#define M_VERB_IMPERATIVE 37		//������������� �����, � ��������
#define M_PARTICIPLE_PASSIVE 39		//������������� �����, � ���������
#define M_REDUCED 40				//���������, � �������������� � ���������

enum M_PERFECT						//�������������, � ��������, ���������, ������������
{
	perfect = 42,
	imperfect = 43
};

#define M_REFLEXIVE 45				//�������������� �� ����, � ��������, ���������, ������������

enum M_ADVERB						//���� �������
{
	definitive = 24,
	circumstantial = 59,
	itterogative = 66
};

enum M_DEFINITIVE_ADVERB			//���� ��������������� �������
{
	qualitative = 25,
	grade = 46,
	method = 50
};

enum M_CIRCUMSTANTIAL_ADVERB		//���� ������������������ �������
{
	time = 60,
	place = 61,
	direction = 62,
	quantity = 63,
	reason = 65,
	goal = 67
};

enum M_COMPARATIVE					//������������� �������, � ��������������, ���������, �������, ������������
{
	comparative = 47,
	superlative = 48
};

enum M_NUMERAL						//���� ������������
{
	ordinal = 58,
	collective = 64,
	indefinite = 68
};

#endif