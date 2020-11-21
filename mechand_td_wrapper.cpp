#include "mechand.h"

#define MECHAN_DATABASE_DIR "/home/project/mechan/database"
#define MECHAN_PIPE_DIR "/tmp/mechan"
#define MECHAN_ID 1156458603
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
	
class TdWrapper
{
private:
	bool _active = false;
	int _readpipe = -1;
	Mechan *_mechan = nullptr;

	std::int64_t _dreadnumber(char delim);
	void _processmessagetext();
	void _processmessage();
	unsigned int _processmessages();

	//friends to Mechan
	void send(std::int64_t chatid, const char *text); friend Mechan;

public:
	TdWrapper();
	void loop();
	~TdWrapper();
};
	
std::int64_t TdWrapper::_dreadnumber(char delim)
{
	std::int64_t result = 0;
	while(true)
	{
		char c;
		int r = read(_readpipe, &c, 1);
		if (r > 0 && c >= '0' && c <= '9') result = result * 10 + (c - '0');
		else if (c == delim) return result;
		else return 0;
	}
};

void TdWrapper::_processmessagetext()
{
	//Reading delimiter
	char delim;
	if (read(_readpipe, &delim, 1) < 1 || delim != ' ') return;

	//Reading chatid:len:name
	std::int64_t chatid = _dreadnumber(':');
	if (chatid == 0) return;
	unsigned int chatlen = _dreadnumber(':');
	if (chatlen == 0) return;
	char chatname[64];
	if (read(_readpipe, chatname, chatlen) < chatlen) return;
	chatname[chatlen] = '\0';
	
	//Reading delimiter
	if (read(_readpipe, &delim, 1) < 1 || delim != ' ') return;
	
	//Reading userid:len:name
	unsigned int userid = _dreadnumber(':');
	if (userid == 0) return;
	unsigned int userlen = _dreadnumber(':');
	if (userlen == 0) return;
	char username[64];
	if (read(_readpipe, username, userlen) < userlen) return;
	username[userlen] = '\0';
	
	//Reading delimiter
	if (read(_readpipe, &delim, 1) < 1 || delim != ' ') return;
	
	//Read len:message
	unsigned int messagelen = _dreadnumber(':');
	std::string message(messagelen, '\0');
	if (read(_readpipe, &message[0], messagelen) < messagelen) return;
	
	//If not TdWrapper -> write message to tdlib
	if (userid != MECHAN_ID)
	{
		_mechan->process(chatid, message);
	}
}

void TdWrapper::_processmessage()
{
	char messagetype;
	if (read(_readpipe, &messagetype, 1) <= 0) return;
	
	switch (messagetype)
	{
	case 't':
		_processmessagetext();
		break;
	case 'q':
		_active = false;
		break;
	}
};

unsigned int TdWrapper::_processmessages()
{
	unsigned int processed = 0;
	while(_active)
	{
		char symbol;
		int r = read(_readpipe, &symbol, 1);
		if (r > 0)
		{
			if (symbol == '!')
			{
				_processmessage();
				processed++;
			}
		}
		else return processed;
	}
	return processed;
};

void TdWrapper::send(std::int64_t chatid, const char *message)
{
	int writepipe = open(MECHAN_PIPE_DIR "/response", O_WRONLY | O_NONBLOCK);
	if (writepipe >= 0)
	{
		dprintf(writepipe, "!t %lli %u:%s\n", chatid, strlen(message), message);
		close(writepipe);
	}
};

TdWrapper::TdWrapper()
{
	mkdir(MECHAN_PIPE_DIR "", 0755);
	mkfifo(MECHAN_PIPE_DIR "/message", 0644);
	mkfifo(MECHAN_PIPE_DIR "/response", 0644);
	_readpipe = open(MECHAN_PIPE_DIR "/message", O_RDONLY | O_NONBLOCK);
	if (_readpipe < 0) return;
	_mechan = new Mechan(&_active, this);
};

void TdWrapper::loop()
{
	while(_active)
	{
		_processmessages();
		sleep(1);
	}
};

TdWrapper::~TdWrapper()
{
	if (_readpipe > 0) close(_readpipe);
	if (_mechan != nullptr) delete _mechan;
};

void _main()
{
	TdWrapper wrapper;
	wrapper.loop();
};

int main()
{
	_main();
	return 0;
};