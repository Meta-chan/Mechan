#define IR_INCLUDE 'i'
#include <ir/print.h>
#include "../header/mechan.h"
#include "../header/mechan_parse.h"

mechan::Mechan::Mechan() noexcept :
	_dialog(),
	_word(&_dialog),
	_neuro(&_dialog, &_word, false),
	_core(&_dialog, &_word)
{
}

void mechan::Mechan::print_event_log(const std::string string) noexcept
{
	printf("%s\n", string.data());
	if (_event_log_messages > 0)
	{
		if (!_server.send(_event_log_address, string)) _event_log_messages = 0;
		else _event_log_messages--;
	}
}

int mechan::Mechan::main() noexcept
{
	if(!_server.ok()
	|| !_neuro.ok()
	|| !_word.ok()
	|| !_dialog.ok()
	|| !_core.ok()) return 1;

	while (true)
	{
		//Reading and answering
		while (true)
		{
			TCPAddress address;
			std::string message;
			//No message
			if (!_server.receive(&address, &message)) break;
			//Empry message
			else if (message.empty()) {}
			//Non-command
			else if (message.front() != '?' && message.front() != '!')
			{	
				_server.send(address, _core.answer(message));
			}
			else
			{
				std::vector<std::string> parsed;
				parse_space(message, &parsed);
				// !coefficient
				if (message.front() == '!'
					&& parsed.size() == 2
					&& parsed[0] == "coefficient"
					&& strtod(parsed[1].data(), nullptr) > 0)
				{
					_neuro.set_coefficient(strtof(parsed[1].data(), nullptr));
					_server.send(address, "!");
				}
				// ?coefficient
				else if (message.front() == '?'
					&& parsed.size() == 1
					&& parsed[0] == "coefficient")
				{
					char strcoef[32];
					ir::print(strcoef, 32, "%lf", _neuro.get_coefficient());
					_server.send(address, strcoef);
				}
				// !shutdown
				else if (message.front() == '!'
					&& parsed.size() == 1
					&& parsed[0] == "shutdown")
				{
					_server.send(address, "!");
					return 0;
				}
				// !log
				else if (message.front() == '!'
					&& parsed.size() == 2
					&& parsed[0] == "log"
					&& strtol(parsed[1].data(), nullptr, 10) > 0)
				{
					_event_log_address = address;
					_event_log_messages = strtol(parsed[1].data(), nullptr, 10);
					_server.send(address, "!");
				}
				// ?
				else if (message.front() == '?'
					&& parsed.size() == 0)
				{
					_server.send(address, "!");
				}
				// Invalid
				else _server.send(address, "?");
			}
		}

		//Running idle
		clock_t c = clock();
		while (clock() - c < 10 * CLOCKS_PER_SEC) _neuro.train();
	}

	return 0;
}

int _main()
{
	mechan::Mechan mechan;
	return mechan.main();
}

int main()
{
	return _main();
}