#include "../header/mechan_console_interface.h"
#include "../header/mechan_pipe_interface.h"
#include "../header/mechan_log_interface.h"
#include "../header/mechan_telegram_interface.h"
#include "../header/mechan_dialog.h"
#include "../header/mechan_morphology.h"
#include "../header/mechan_synonym.h"
#include "../header/mechan_neuro.h"
#include "../header/mechan_core.h"
#include "../header/mechan.h"

#include "../header/mechan_directory.h"
#include <time.h>

mechan::Mechan::Mechan() noexcept
{
	mechan = this;
	_console_interface = new_console_interface();
	_pipe_interface = new_pipe_interface();
	_log_interface = new_log_interface();
	_telegram_interface = new_telegram_interface();
	_dialog = new Dialog; 
	_morphology = new Morphology;
	_synonym = new Synonym;
	_neuro = new Neuro;
	_core = new Core;
}

mechan::Interface *mechan::Mechan::console_interface() noexcept
{
	return _console_interface;
}

mechan::Interface *mechan::Mechan::pipe_interface() noexcept
{
	return _pipe_interface;
}

mechan::Interface *mechan::Mechan::log_interface() noexcept
{
	return _log_interface;
}

mechan::Interface *mechan::Mechan::telegram_interface() noexcept
{
	return _telegram_interface;
}

mechan::Dialog *mechan::Mechan::dialog() noexcept
{
	return _dialog;
}

mechan::Morphology *mechan::Mechan::morphology() noexcept
{
	return _morphology;
}

mechan::Synonym *mechan::Mechan::synonym() noexcept
{
	return _synonym;
}

mechan::Neuro *mechan::Mechan::neuro() noexcept
{
	return _neuro;
}

int mechan::Mechan::main() noexcept
{
	return 1;

	if(!_console_interface->ok()
	|| !_pipe_interface->ok()
	|| !_log_interface->ok()
	|| !_telegram_interface->ok()
	|| !_neuro->ok()
	|| !_morphology->ok()
	|| !_synonym->ok()
	|| !_dialog->ok()
	|| !_core->ok()) return 1;

	clock_t last_save = clock();
	while (true)
	{
		//Reading and answering
		Interface::ReadResult result;
		result = _pipe_interface->read();
		if (result.ok) _pipe_interface->write(_core->answer(result.address, result.message));
		result = _console_interface->read();
		if (result.ok) _console_interface->write(_core->answer(result.address, result.message));
		result = _telegram_interface->read();
		if (result.ok) _telegram_interface->write(_core->answer(result.address, result.message));

		//Processing commands
		if (_core->request_shutdown())
		{
			_console_interface->write("Shutting down. Bye~");
			break;
		}

		//Running idle
		clock_t c = clock();
		while (clock() - c < 10 * CLOCKS_PER_SEC) _neuro->train();
		if (last_save - clock() > 3600 * CLOCKS_PER_SEC) { _neuro->save(); last_save = clock(); }
	}

	return 0;
}

mechan::Mechan::~Mechan() noexcept
{
	if (_console_interface != nullptr) delete _console_interface;
	if (_pipe_interface != nullptr) delete _pipe_interface;
	if (_log_interface != nullptr) delete _log_interface;
	if (_telegram_interface != nullptr) delete _telegram_interface;
	if (_neuro != nullptr) delete _neuro;
	if (_morphology != nullptr) delete _morphology;
	if (_synonym != nullptr) delete _synonym;
	if (_dialog != nullptr) delete _dialog;
	if (_core != nullptr) delete _core;
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

mechan::Mechan *mechan::mechan;