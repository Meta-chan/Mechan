#include "../header/mechan.h"
#include "../header/mechan_console_interface.h"
#include "../header/mechan_pipe_interface.h"
#include "../header/mechan_log_interface.h"
#include "../header/mechan_telegram_interface.h"
#include "../header/mechan_morphology.h"
#include "../header/mechan_neuro.h"
#include "../header/mechan_dialog.h"
#include "../header/mechan_synonym.h"

mechan::Mechan::Mechan() noexcept
{
	_console_interface = new_console_interface();
	_pipe_interface = new_pipe_interface();
	_log_interface = new_log_interface();
	_telegram_interface = new_telegram_interface();
	_morphology = new Morphology;
	_neuro = new Neuro;
	_dialog = new Dialog;
	_synonym = new Synonym;
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

mechan::Neuro *mechan::Mechan::neuro() noexcept
{
	return _neuro;
}

mechan::Morphology *mechan::Mechan::morphology() noexcept
{
	return _morphology;
}

mechan::Synonym *mechan::Mechan::synonym() noexcept
{
	return _synonym;
}

mechan::Dialog *mechan::Mechan::dialog() noexcept
{
	return _dialog;
}

int mechan::Mechan::main() noexcept
{
	//MAIN CODE HERE!!!
	if(!_console_interface->ok()
	|| !_pipe_interface->ok()
	|| !_log_interface->ok()
	|| !_telegram_interface->ok()
	|| !_neuro->ok()
	|| !_morphology->ok()
	|| !_synonym->ok()
	|| !_dialog->ok()) return 1;

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