#pragma once

#include "../header/mechan_socket.h"
#include "../header/mechan_dialog.h"
#include "../header/mechan_word.h"
#include "../header/mechan_neuro.h"
#include "../header/mechan_core.h"

namespace mechan
{

	class Mechan
	{
	private:
		Server::Address _event_log_address;
		unsigned int _event_log_messages = 0;

		Server _server;
		Dialog _dialog;
		Word _word;
		Neuro _neuro;
		Core _core;

	public:
		Mechan()										noexcept;
		Dialog *dialog()								noexcept;
		Word *word()									noexcept;
		Neuro *neuro()									noexcept;
		void print_event_log(const std::string string)	noexcept;
		int main()										noexcept;
	};
}