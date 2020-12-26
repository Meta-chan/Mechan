#include "../header/mechan.h"
#include "../header/mechan_directory.h"

mechan::Mechan::Mechan() noexcept :
	_dialog(this),
	_morphology(this),
	_synonym(this),
	_neuro(this),
	_core(this)
{
}

mechan::Dialog *mechan::Mechan::dialog() noexcept
{
	return &_dialog;
}

mechan::Morphology *mechan::Mechan::morphology() noexcept
{
	return &_morphology;
}

mechan::Synonym *mechan::Mechan::synonym() noexcept
{
	return &_synonym;
}

mechan::Neuro *mechan::Mechan::neuro() noexcept
{
	return &_neuro;
}

void mechan::Mechan::print_event_log(const std::string string) noexcept
{
	printf("%s\n", string.data());
	if (_event_log_hook)
	{
		if (!_server.send(string, _event_log_address)) _event_log_hook = false;
	}
}

int mechan::Mechan::main() noexcept
{
	if(!_server.ok()
	|| !_neuro.ok()
	|| !_morphology.ok()
	|| !_synonym.ok()
	|| !_dialog.ok()
	|| !_core.ok()) return 1;

	while (true)
	{
		//Reading and answering
		while (true)
		{
			Server::ReceiveResult result = _server.receive();
			if (!result.ok) break;
			else if (result.message == "!shutdown")
			{
				print_event_log("Shutdown...");
				return 0;
			}
			else if (result.message == "!eventlog")
			{
				_event_log_address = result.address;
				_event_log_hook = true;
			}
			else _server.send(_core.answer(result.message), result.address);
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