#include "../header/mechan.h"
#include "../header/mechan_parse.h"

mechan::Mechan::Mechan() noexcept :
	_dialog(this),
	_word(this),
	_neuro(this),
	_core(this)
{
}

mechan::Dialog *mechan::Mechan::dialog() noexcept
{
	return &_dialog;
}

mechan::Word *mechan::Mechan::word() noexcept
{
	return &_word;
}

mechan::Neuro *mechan::Mechan::neuro() noexcept
{
	return &_neuro;
}

void mechan::Mechan::print_event_log(const std::string string) noexcept
{
	printf("%s\n", string.data());
	if (_event_log_messages > 0)
	{
		if (!_server.send(string, _event_log_address)) _event_log_messages = 0;
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
			Server::ReceiveResult result = _server.receive();
			//No message
			if (!result.ok) break;
			//Empry message
			else if (result.message.empty()) {}
			//Non-command
			else if (result.message.front() != '?' && result.message.front() != '!')
			{	
				_server.send(_core.answer(result.message), result.address);
			}
			else
			{
				std::vector<std::string> parsed;
				parse_space(result.message, &parsed);
				// !coefficient
				if (result.message.front() == '!'
					&& parsed.size() == 2
					&& parsed[0] == "coefficient"
					&& strtod(parsed[1].data(), nullptr) > 0)
				{
					_neuro.set_coefficient(strtod(parsed[1].data(), nullptr));
					_server.send("!", result.address);
				}
				// ?coefficient
				else if (result.message.front() == '?'
					&& parsed.size() == 1
					&& parsed[0] == "coefficient")
				{
					char strcoef[32];
					sprintf(strcoef, "%lf", _neuro.get_coefficient());
					_server.send(strcoef, result.address);
				}
				// !shutdown
				else if (result.message.front() == '!'
					&& parsed.size() == 1
					&& parsed[0] == "shutdown")
				{
					_server.send("!", result.address);
					return 0;
				}
				// !log
				else if (result.message.front() == '!'
					&& parsed.size() == 2
					&& parsed[0] == "log"
					&& strtol(parsed[1].data(), nullptr, 10) > 0)
				{
					_event_log_address = result.address;
					_event_log_messages = strtol(parsed[1].data(), nullptr, 10);
					_server.send("!", result.address);
				}
				// ?
				else if (result.message.front() == '?'
					&& parsed.size() == 0)
				{
					_server.send("!", result.address);
				}
				// Invalid
				else _server.send("?", result.address);
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