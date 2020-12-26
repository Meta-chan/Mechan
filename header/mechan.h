#pragma once

#include "../header/mechan_socket.h"
#include "../header/mechan_dialog.h"
#include "../header/mechan_morphology.h"
#include "../header/mechan_synonym.h"
#include "../header/mechan_neuro.h"
#include "../header/mechan_core.h"

namespace mechan
{

	class Mechan
	{
	private:
		Server::Address _event_log_address;
		bool _event_log_hook = false;

		Server _server;
		Dialog _dialog;
		Morphology _morphology;
		Synonym _synonym;
		Neuro _neuro;
		Core _core;

	public:
		Mechan()										noexcept;
		Dialog *dialog()								noexcept;
		Morphology *morphology()						noexcept;
		Synonym *synonym()								noexcept;
		Neuro *neuro()									noexcept;
		void print_event_log(const std::string string)	noexcept;
		int main()										noexcept;
	};
}