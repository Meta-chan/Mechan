//Dependencies inside project
#include "header/lowercase_char.h"

//Dependencies on OS
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#include <share.h>
	#ifndef _DEBUG
		#define NDEBUG
	#endif
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
#endif

//Dependencies on C++ library
#include <random>
#include <time.h>
#include <stdio.h>

//Dependencies on Ironic
#define IR_IMPLEMENT
#define IR_NEURO_CRITICAL_OPENMP
#include <ir_syschar.h>
#include <ir_neuro.h>
#include <ir_database/ir_n2st_database.h>

//Mechan foledrs
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define MECHAN_DATABASE_DIR L"C:\\Project\\mechan\\database"
	#define MECHAN_SAVE_DIR L"C:\\Project\\mechan\\database"
	#define MECHAN_LOG_FILE L"C:\\Project\\mechan\\log.txt"
#else
	#define MECHAN_DATABASE_DIR "/home/project/mechan/database"
	#define MECHAN_PIPE_DIR "/tmp/mechan"
	#define MECHAN_SAVE_DIR "/home/project/mechan/database"
	#define MECHAN_LOG_FILE "/home/project/mechan/log.txt"
#endif

//Mechan settings
#define N_WORDS 10
#define N_CHARS 10
#define MESSAGE_SIZE (33 * N_CHARS * N_WORDS + 3)
#define INPUT_SIZE (2 * MESSAGE_SIZE)
#define N_NEGATIVE 1
#define TRAIN_PART 0.98
#define SECONDS_BETWEEN_SAVES 3600
#define TYPE double
#define ALIGN 2
#define COEF 0.1

class MechanTrainer
{
private:
	enum class Message
	{
		no,
		stop
	};

	bool _ok = false;
	ir::N2STDatabase *_dialog = nullptr;
	ir::Neuro<TYPE, ALIGN, ir::Tanh<TYPE>> *_neuro = nullptr;
	std::default_random_engine _generator;
	std::uniform_int_distribution<unsigned int> *_distribution = nullptr;
	clock_t _lasttest = 0;
	clock_t _lastsave = 0;
	clock_t _traincount = 0;

	#ifndef _WIN32 
		int _readpipe = -1;
	#endif

	Message _get_user_message();
	void _write_user_message(const char *s);
	void _quasi_random_message(unsigned int nmessage, unsigned int intseed, ir::ConstBlock strseed, ir::ConstBlock *result);
	void _unroll_char(unsigned char c, TYPE v[33]);
	const char *_unroll_word(const char *databegin, const char *end, TYPE v[33 * N_CHARS]);
	void _unroll_message(const char *databegin, const char *end, TYPE v[MESSAGE_SIZE]);
	void _train();
	unsigned int _test();

public:
	MechanTrainer(bool *ok);
	void loop();
	~MechanTrainer();
};

MechanTrainer::MechanTrainer(bool *ok)
{
	//opening database
	ir::ec code;
	_dialog = new ir::N2STDatabase(MECHAN_DATABASE_DIR SS("/dialog.idt"), ir::Database::createmode::create_read, &code);
	if (code != ir::ec::ec_ok) return;
	_dialog->set_ram_mode(true, true);

	//loading ir::Neuro
	unsigned int layers[4] = {INPUT_SIZE, 2000, 2000, 1 };
	_neuro = new ir::Neuro<TYPE, ALIGN>(MECHAN_SAVE_DIR SS("/dialog.inr"), &code);
	if (code != ir::ec::ec_ok)
	{
		delete _neuro;
		_neuro = new ir::Neuro<TYPE, ALIGN>(4, layers, 0.002, &code);
		if (code != ir::ec::ec_ok) return;
	}
	_neuro->set_coefficient(COEF);
	
	//Creating distribution
	_generator.seed((unsigned int)time(nullptr));
	_distribution = new std::uniform_int_distribution<unsigned int>(0, (unsigned int)(TRAIN_PART * _dialog->get_table_size()) - 2);

	//Setting clock
	_lastsave = clock();
	
	//Opening pipes
	#ifndef _WIN32
		mkdir(MECHAN_PIPE_DIR, 0775);
		mkfifo(MECHAN_PIPE_DIR "/neuro_input", 0664);
		mkfifo(MECHAN_PIPE_DIR "/neuro_output", 0664);
		_readpipe = open(MECHAN_PIPE_DIR "/neuro_input", O_RDONLY | O_NONBLOCK);
		if (_readpipe < 0) return;
	#endif

	if (ok != nullptr) *ok = true;
	_ok = true;
};

MechanTrainer::Message MechanTrainer::_get_user_message()
{
	#ifdef _WIN32
		return Message::no;
	#else
		char c;
		if (read(_readpipe, &c, 1) == 1 && c == '!') return Message::stop;
		else return Message::no;
	#endif
};

void MechanTrainer::_write_user_message(const char *s)
{
	#ifdef _WIN32
		printf("%s", s);
	#else
		int writepipe = open(MECHAN_PIPE_DIR "/neuro_output", O_WRONLY | O_NONBLOCK);
		if (writepipe < 0) return;
		dprintf(writepipe, "%s", s);
		close(writepipe);
	#endif
};

void MechanTrainer::_quasi_random_message(unsigned int nmessage, unsigned int intseed, ir::ConstBlock strseed, ir::ConstBlock *result)
{
	//FNV-1a
	unsigned int random = 2166136261;
	for (unsigned int i = 0; i < strseed.size; i++)
	{
		unsigned int octet = ((char*)strseed.data)[i];
		random = random ^ octet;
		random = random * 16777619;
	}
	random = random ^ intseed;
	random = random * 16777619;

	unsigned int begin = (unsigned int)(TRAIN_PART * _dialog->count());
	random = begin + random % (_dialog->count() - begin);
	while ((random == nmessage) || _dialog->read(random, result) != ir::ec::ec_ok)
	{
		random++;
		if (random > _dialog->count()) random = begin;
	}
};

void MechanTrainer::_unroll_char(unsigned char c, TYPE v[33])
{
	unsigned int nchar = (c >= 0xE0) ? (c - 0xE0) : 32;
	for (unsigned int i = 0; i < nchar; i++) v[i] = -1;
	v[nchar] = 1;
	for (unsigned int i = nchar + 1; i < 33; i++) v[i] = -1;
};

const char *MechanTrainer::_unroll_word(const char *databegin, const char *end, TYPE v[33 * N_CHARS])
{
	unsigned int nletter = 0;
	while (end >= databegin)
	{
		bool isletter;
		char c = lowercase_char(*end, &isletter);
		end--;
		if (isletter || c == '-')
		{
			if (nletter < N_CHARS)_unroll_char(c, v + 33 * nletter);
			nletter++;
		}
		else break;
	}

	while (nletter < N_CHARS)
	{
		_unroll_char(0, v + 33 * nletter);
		nletter++;
	}

	return end;
};

void MechanTrainer::_unroll_message(const char *databegin, const char *end, TYPE v[MESSAGE_SIZE])
{	
	char lastchar = *end;
	
	unsigned int nwords = 0;
	while (end >= databegin)
	{
		bool isletter;
		char c = lowercase_char(*end, &isletter);
		if (isletter)
		{
			if (nwords < N_WORDS) end = _unroll_word(databegin, end, v + 33 * N_CHARS * nwords);
			else break; //nothing else can happen
			nwords++;
		}
		else end--;
	}

	while (nwords < N_WORDS)
	{
		_unroll_word(databegin, databegin - 1, v + 33 * N_CHARS * nwords);
		nwords++;
	}

	v[33 * N_WORDS * N_CHARS] = (lastchar == '.') ? 1.0 : -1.0;
	v[33 * N_WORDS * N_CHARS + 1] = (lastchar == '?') ? 1.0 : -1.0;
	v[33 * N_WORDS * N_CHARS + 2] = (lastchar == '!') ? 1.0 : -1.0;
};

void MechanTrainer::_train()
{
	//unrolling message
	unsigned int nmessage = (*_distribution)(_generator);
	ir::ConstBlock message;
	
	if (_dialog->probe(nmessage + 1) != ir::ec::ec_ok || _dialog->read(nmessage, &message) != ir::ec::ec_ok) return;
	char *text_message = (char*)message.data;
	_unroll_message(text_message, text_message + message.size - 1, _neuro->get_input()->data());
	
	if (((*_distribution)(_generator) % (1 + N_NEGATIVE)) == 0)
	{
		//positive training
		if (_dialog->read(nmessage + 1, &message) != ir::ec::ec_ok) return;
		text_message = (char*)message.data;
		_unroll_message(text_message, text_message + message.size - 1, _neuro->get_input()->data() + MESSAGE_SIZE);
		_neuro->forward();
		_neuro->get_goal()->data()[0] = 1.0;
		_neuro->backward();
		char writemsg[512];
		sprintf(writemsg, "Performed positive training %u, result: %lf\n", (unsigned int)_traincount, _neuro->get_output()->data()[0]);
		_write_user_message(writemsg);
		_traincount++;
	}
	else
	{
		//negative training
		unsigned int i = 0;
		while (i < N_NEGATIVE)
		{
			nmessage = (*_distribution)(_generator);
			if (_dialog->read(nmessage, &message) == ir::ec_ok)
			{
				text_message = (char*)message.data;
				_unroll_message(text_message, text_message + message.size - 1, _neuro->get_input()->data() + MESSAGE_SIZE);
				_neuro->forward();
				_neuro->get_goal()->data()[0] = -1.0;
				_neuro->backward();
				char writemsg[512];
				sprintf(writemsg, "Performed negative training %u, result: %lf\n", (unsigned int)_traincount, _neuro->get_output()->data()[0]);
				_write_user_message(writemsg);
				_traincount++;
				i++;
			}
		}
	}
};

unsigned int MechanTrainer::_test()
{
	unsigned int count = 0, rightcount = 0;
	const unsigned int begin = (unsigned int)(TRAIN_PART * _dialog->count());
	ir::ConstBlock message;
	for (unsigned int nmessage = begin; nmessage < _dialog->count(); nmessage++)
	{
		//true forward
		if (_dialog->probe(nmessage + 1) != ir::ec::ec_ok || _dialog->read(nmessage, &message) != ir::ec::ec_ok) continue;
		_unroll_message((char*)message.data, (char*)message.data + message.size - 1, _neuro->get_input()->data());
		if (_dialog->read(nmessage + 1, &message) != ir::ec::ec_ok) continue;
		_unroll_message((char*)message.data, (char*)message.data + message.size - 1, _neuro->get_input()->data() + MESSAGE_SIZE);
		_neuro->forward();
		TYPE reply_output = _neuro->get_output()->data()[0];

		//fake forward
		bool right = true;
		for (unsigned int i = 0; i < N_NEGATIVE; i++)
		{
			_quasi_random_message(nmessage, i, message, &message);
			_unroll_message((char*)message.data, (char*)message.data + message.size - 1, _neuro->get_input()->data() + MESSAGE_SIZE);
			_neuro->forward();
			TYPE random_output = _neuro->get_output()->data()[0];
			if (random_output >= reply_output) { right = false; break; }
		}

		if (right) rightcount++;
		count++;

		char writemsg[512];
		sprintf(writemsg, "Possible tests: %u, performed: %u, successfull: %u, best: %u\n",
			(unsigned int)(_dialog->count() - begin),
			count,
			rightcount,
			_lasttest);
		_write_user_message(writemsg);
	}

	#ifdef _WIN32
		FILE *file = _wfsopen(MECHAN_LOG_FILE, L"a", _SH_DENYNO);
	#else
		FILE *file = fopen(MECHAN_LOG_FILE, "a");
	#endif
	if (file != nullptr)
	{
		fprintf(file, "Possible tests: %u, performed: %u, successfull: %u, best: %u\n",
			(unsigned int)(_dialog->count() - begin),
			count,
			rightcount,
			_lasttest);
		fclose(file);
	}
	return rightcount;
};

void MechanTrainer::loop()
{
	if (!_ok) return;

	_lasttest = _test();
	
	while (true)
	{
		_train();
		if (_get_user_message() == Message::stop) break;
		
		if ((clock() - _lastsave) / CLOCKS_PER_SEC > (SECONDS_BETWEEN_SAVES))
		{
			_traincount = 0;
			unsigned int newtest = _test();
			if (newtest == 0 || newtest > 13000)
			{
				break;
			}
			else
			{
				_lasttest = newtest;
				_neuro->save(MECHAN_SAVE_DIR SS("/dialog.inr"));
				_lastsave = clock();
			}
		}
	}
};

MechanTrainer::~MechanTrainer()
{
	if (_distribution != nullptr) delete _distribution;
	if (_neuro != nullptr) delete _neuro;
	if (_dialog != nullptr) delete _dialog;
	#ifndef _WIN32
		if (_readpipe >= 0) close(_readpipe);
	#endif
};

int main()
{
	bool ok = false;
	MechanTrainer *trainer = new MechanTrainer(&ok);
	if (ok) trainer->loop();
	delete trainer;
	return 0;
};
