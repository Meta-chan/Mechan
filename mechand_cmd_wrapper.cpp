#define MECHAN_DATABASE_DIR L"C:\\Project\\mechan\\database"

#define IR_IMPLEMENT
#include "header/mechand.h"

class CmdWrapper
{
private:
	Mechan *_mechan = nullptr;
	bool _ok = false;

	static void _callback(void *user, std::int64_t chatid, const std::string message);

public:
	CmdWrapper();
	void loop();
	
	~CmdWrapper();
};

void CmdWrapper::_callback(void *user, std::int64_t chatid, std::string message)
{
	printf("%s", (char*)utf_buffer_recode(&utf_utf8, message.c_str(), ' ', &utf_866));
};

CmdWrapper::CmdWrapper()
{
	utf_init();
	utf_866.init();
	utf_1251.init();
	utf_utf8.init();
	_mechan = new Mechan(&_callback, nullptr, &_ok);
};

void CmdWrapper::loop()
{
	while (true)
	{
		std::string s866;
		std::getline(std::cin, s866);
		if (strstr(s866.c_str(), "exit") != nullptr) return;
		_mechan->process(0, (const char *)utf_buffer_recode(&utf_866, s866.c_str(), ' ', &utf_utf8));
	}
};

CmdWrapper::~CmdWrapper()
{
	if (_mechan != nullptr) delete _mechan;
};

void _main()
{
	CmdWrapper wrapper;
	wrapper.loop();
};

int main()
{
	_main();
	return 0;
};