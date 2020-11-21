#include <stdio.h>
#include <vector>
#include <ir_database/ir_n2st_database.h>

class DialogExtracter
{
private:
	enum State
	{
		plain_text_wait_line,
		plain_text_wait_dash,

		direct_speech_wait_dot,
		direct_speech_wait_dash,

		author_speech_wait_dot,
		author_speech_wait_dash,

		corrupted_wait_line
	};
	
	State _state = plain_text_wait_line;
	unsigned int _directcount = 0;
	unsigned int _authorcount = 0;
	unsigned int _plaincount = 0;
	bool _firstdialog = true;
	std::vector<unsigned char> _buffer;
	ir::N2STDatabase *_base;

	void _plain_text_wait_line(unsigned char c);
	void _plain_text_wait_dash(unsigned char c);

	void _direct_speech_wait_dot(unsigned char c);
	void _direct_speech_wait_dash(unsigned char c);

	void _author_speech_wait_dot(unsigned char c);
	void _author_speech_wait_dash(unsigned char c);

	void _corrupred_wait_line(unsigned char c);
	
	void _push_char(unsigned char c); 
	void _push_buffer();

public:
	void parse(const char *text, ir::N2STDatabase *base);
};

#include "../implementation/dialog_extracter.h"