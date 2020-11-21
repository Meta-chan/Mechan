#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <vector>
#include <string>
#include <math.h>

#include <ir_utf.h>
#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>

class Mechan
{
public:
	typedef void Callback(void *user, std::int64_t chatid, const std::string message);

private:
	struct Vector
	{
		float squaresum;
		std::vector<float> vector;
	};

	ir::N2STDatabase *_dialog	= nullptr;
	ir::S2STDatabase *_vectors	= nullptr;
	Callback *_callback			= nullptr;
	void *_user					= nullptr;
	bool _ok					= false;

	void _parse(const std::string s, std::vector<std::string> *words);
	float _multiply(const Vector *v1, const Vector *v2);
	void _readvector(const std::string s, Vector *v);
	float _heuristic(const std::string s, const std::vector<std::string> *messagewords, const std::vector<Vector> *messagevectors);

public:
	Mechan(Callback *callback, void *user, bool *ok);
	void process(std::int64_t chatid, const std::string message);
	~Mechan();
};

#include "../implementation/mechand.h"