#ifndef MORPHOLOGY
#define MORPHOLOGY

#define M_DEPRECATED 255

enum M_TYPE
{
	noun = 1,			//существительное
	adjective = 14,		//прилагательное
	conjunktion = 17,	//союз
	interjection = 18,	//междометие
	predicate = 19,		//предикатив (наречие, которое может выступать подлежащим)
	particle = 20,		//частица (несЄт смысл и содержание, не вли€ет на грамматику)
	preposition = 22,	//предлог
	adverb = 23,		//наречие
	verb = 27,			//глагол
	participle = 38,	//причастие (прилагательное, которое отвечает на вопросы глагола)
	transgressive = 41,	//деепричастие (наречие, отвечающее на вопросы глагола)
	pronoun = 51,		//местоимение
	introductory = 52,	//вводное слово
	numeral = 57		//числитель
};

enum M_ANIMATE			//есть у существительных, прилагательных, причастий. Ћучше игнорировать.
{
	inanimate = 2,
	animate = 15
};

enum M_NUMBER			//есть у существительных, иногда глаголов, причастий, прилагательных
{
	singular = 3,
	plural = 13
};

enum M_GENDER			//≈сть у существительных, иногда глаголов, причастий, прилагательных, собирательных числителей, местоимений
{
	feminine = 4,
	maskuline = 6,
	neutrum = 16,
	common = 26
};

enum M_CASE				//≈сть у существительных, причастий, прилагательных, числительных, местоимений
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

#define M_INDECLINABLE_NOUN	5		//Ќесклон€емое, только у существительных
#define M_UNCHANGABLE_ADJECTIVE 21	//Ќеизмен€емое, только у прилагательных

#define M_VERB_SECOND_CONJUGATION 28//¬торой тип спр€жени€, у глаголов, причастий, деепричастий

enum M_TRANSITIVITY					//ѕереходность, у глаголов, причастий, деепричатсий
{
	transitive = 29,
	intransitive = 44,
	context = 49
};

#define M_VERB_INFINITIVE 30		//»нфинитив, у глаголов

enum M_TENSE						//¬рем€, есть у глаголов, причастий, деепричастий
{
	past = 31,
	present = 32,
	future = 36,
};

enum M_PERSON						//Ћицо, у глаголов
{
	first = 34,
	second = 35,
	third = 33
};

#define M_VERB_IMPERATIVE 37		//ѕоселительный залог, у глаголов
#define M_PARTICIPLE_PASSIVE 39		//—традательный залог, у причастий
#define M_REDUCED 40				// раткость, у прилагательных и причастий

enum M_PERFECT						//—овершЄнность, у глаголов, причастий, деепричастий
{
	perfect = 42,
	imperfect = 43
};

#define M_REFLEXIVE 45				//Ќаправленность на себ€, у глаголов, причастий, деепричастий

enum M_ADVERB						//“ипы наречий
{
	definitive = 24,
	circumstantial = 59,
	itterogative = 66
};

enum M_DEFINITIVE_ADVERB			//“ипы определительных наречий
{
	qualitative = 25,
	grade = 46,
	method = 50
};

enum M_CIRCUMSTANTIAL_ADVERB		//“ипы обсто€тельственных наречий
{
	time = 60,
	place = 61,
	direction = 62,
	quantity = 63,
	reason = 65,
	goal = 67
};

enum M_COMPARATIVE					//—равнительные степени, у прилагательных, причастий, наречий, деепричастий
{
	comparative = 47,
	superlative = 48
};

enum M_NUMERAL						//“ипы числительных
{
	ordinal = 58,
	collective = 64,
	indefinite = 68
};

#endif