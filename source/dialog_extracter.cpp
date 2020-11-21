#include <ir_utf.h>

#define MAX_DIRECT_SPEECH 300
#define MAX_AUTHOR_SPEECH 300
#define MAX_PLAIN_TEXT 300

void DialogExtracter::_plain_text_wait_line(unsigned char c)
{
	_plaincount++;
	switch (c)
	{
	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		break;
	}
};

void DialogExtracter::_plain_text_wait_dash(unsigned char c)
{
	_plaincount++;
	switch (c)
	{
	case '\n':
	case '\r':
	case '\t':
	case ' ':
		break;

	case 0x96: //m dash
	case 0x97: //n dash
		_state = State::direct_speech_wait_dot;
		break;

	default:
		_state = State::plain_text_wait_line;
	}
};

void DialogExtracter::_direct_speech_wait_dot(unsigned char c)
{
	switch (c)
	{
	case '.':
	case 0x85:
	case ',':
	case '!':
	case '?':
		_state = State::direct_speech_wait_dash;
		_push_char(c);
		break;

	case '\r':
	case '\n':
		_state = State::plain_text_wait_dash;
		_push_buffer();
		break;

	default:
		_push_char(c);
	}

	if (_directcount++ == MAX_DIRECT_SPEECH) _state = State::corrupted_wait_line;
};

void DialogExtracter::_direct_speech_wait_dash(unsigned char c)
{
	switch (c)
	{
	case '.':
	case 0x85:
	case ',':
	case '!':
	case '?':
	case ' ':
	case '\t':
		_push_char(c);
		break;

	case 0x96:
	case 0x97:
		_state = State::author_speech_wait_dot;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_push_buffer();
		break;

	default:
		_state = State::direct_speech_wait_dot;
		_push_char(c);
	}
	if (_directcount++ == MAX_DIRECT_SPEECH) _state = State::corrupted_wait_line;
};

void DialogExtracter::_author_speech_wait_dot(unsigned char c)
{
	switch (c)
	{
	case '.':
	case 0x85:
	case ',':
	case '!':
	case '?':
		_state = State::author_speech_wait_dash;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_push_buffer();
		break;
	}
	if (_authorcount++ == MAX_AUTHOR_SPEECH) _state = State::corrupted_wait_line;
};

void DialogExtracter::_author_speech_wait_dash(unsigned char c)
{
	switch (c)
	{
	case '.':
	case 0x85:
	case ',':
	case '!':
	case '?':
	case ' ':
	case '\t':
		break;

	case 0x96:
	case 0x97:
		_state = State::direct_speech_wait_dot;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_push_buffer();
		break;

	default:
		_state = State::author_speech_wait_dot;
	}
	if (_authorcount++ == MAX_AUTHOR_SPEECH) _state = State::corrupted_wait_line;
};

void DialogExtracter::_corrupred_wait_line(unsigned char c)
{
	switch (c)
	{
	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_buffer.resize(0);
		_plaincount = 0;
		_directcount = 0;
		_authorcount = 0;
		break;
	}
};

bool noedgesymbol(unsigned char c)
{
	return (c == ' '	||
		c == '\n'		||
		c == '\r'		||
		c == '\t'		||
		c == 0x96		||
		c == 0x97);
};

bool norepeatsymbol(unsigned char c)
{
	return (c == ' '||
		c == '\t'	||
		c == '\n'	||
		c == '\r'	||
		c == '!'	||
		c == '?'	||
		c == '-'	||
		c == '('	||
		c == ')'	||
		c == 0x96	||
		c == 0x97);
};

void DialogExtracter::_push_char(unsigned char c)
{
	if (_buffer.size() == 0)
	{
		if (!noedgesymbol(c)) _buffer.push_back(c);
	}
	else
	{
		if (_buffer[_buffer.size() - 1] != c || (!norepeatsymbol(c))) _buffer.push_back(c);
	}
};

void DialogExtracter::_push_buffer()
{
	while (_buffer.size() > 0 && noedgesymbol(_buffer[_buffer.size() - 1]))
	{
		_buffer.resize(_buffer.size() - 1);
	}

	if (_buffer.size() > 0)
	{
		if (_plaincount > MAX_PLAIN_TEXT || _firstdialog) _base->insert(_base->get_table_size(), ir::Block(0, nullptr));
		_base->insert(_base->get_table_size(), ir::Block(_buffer.size(), _buffer.data()));
	}

	_firstdialog = false;
	_plaincount = 0;
	_directcount = 0;
	_authorcount = 0;
	_buffer.resize(0);
};

void DialogExtracter::parse(const char *text, ir::N2STDatabase *base)
{
	_base = base;
	_state = State::plain_text_wait_line;
	_plaincount = 0;
	_directcount = 0;
	_authorcount = 0;
	_firstdialog = true;

	while (*text != '\0')
	{
		unsigned char c;
		if (strcmp(text, "<tab>") == 0)
		{
			c = '\t';
			text += 5;
		}
		else if (*text == (char)0xA0 || *text == (char)0xAD || *text == (char)0x98)
		{
			c = ' ';
			text++;
		}
		else
		{
			c = *text;
			text++;
		}

		switch (_state)
		{
			case State::plain_text_wait_line: _plain_text_wait_line(c); break;
			case State::plain_text_wait_dash: _plain_text_wait_dash(c); break;
			case State::direct_speech_wait_dot: _direct_speech_wait_dot(c); break;
			case State::direct_speech_wait_dash: _direct_speech_wait_dash(c);  break;
			case State::author_speech_wait_dot: _author_speech_wait_dot(c); break;
			case State::author_speech_wait_dash: _author_speech_wait_dash(c); break;
			default: _corrupred_wait_line(c);
		}
	}
	if(_state != State::corrupted_wait_line)_push_buffer();
};
