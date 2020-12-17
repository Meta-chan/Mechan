#include "../header/mechan.h"
#include "../header/mechan_directory.h"
#include <time.h>

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
{}

void mechan::Mechan::print_train_log(const std::string string) noexcept
{}

int mechan::Mechan::main() noexcept
{
	return 1;

	if(!_server.ok()
	|| !_neuro.ok()
	|| !_morphology.ok()
	|| !_synonym.ok()
	|| !_dialog.ok()
	|| !_core.ok()) return 1;

	clock_t last_save = clock();
	while (true)
	{
		//Reading and answering
		while (true)
		{
			Server::ReceiveResult result = _server.receive();
			if (!result.ok) break;
			else if (result.message == "!shutdown") return 0;
			else if (result.message == "!eventlog") _event_log_address = result.address;
			else if (result.message == "!neurolog") _neuro_log_address = result.address;
			else _server.send(_core.answer(result.message), result.address);
		}

		//Running idle
		clock_t c = clock();
		while (clock() - c < 10 * CLOCKS_PER_SEC) _neuro.train();
		if (last_save - clock() > 3600 * CLOCKS_PER_SEC) { _neuro.save(); last_save = clock(); }
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