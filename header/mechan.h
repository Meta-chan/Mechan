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
		TCPAddress _event_log_address;
		size_t _event_log_messages = 0;

		TCPServer _server;
		Dialog _dialog;
		Word _word;
		Neuro _neuro;
		Core _core;

	public:
		Mechan()										noexcept;
		void print_event_log(const std::string string)	noexcept;
		int main()										noexcept;
	};
}