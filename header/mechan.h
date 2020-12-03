#pragma once

#include "mechan_interface.h"
#include "mechan_neuro.h"
#include "mechan_morphology.h"
#include "mechan_synonym.h"
#include "mechan_dialog.h"

namespace mechan
{
	class Mechan
	{
	private:
		Interface *_console_interface	= nullptr;
		Interface *_pipe_interface		= nullptr;
		Interface *_log_interface		= nullptr;
		Interface *_telegram_interface	= nullptr;
		Neuro *_neuro					= nullptr;
		Morphology *_morphology			= nullptr;
		Synonym *_synonym				= nullptr;
		Dialog *_dialog					= nullptr;

	public:
		Mechan()						noexcept;
		Interface *console_interface()	noexcept;
		Interface *pipe_interface()		noexcept;
		Interface *log_interface()		noexcept;
		Interface *telegram_interface()	noexcept;
		Neuro *neuro()					noexcept;
		Morphology *morphology()		noexcept;
		Synonym *synonym()				noexcept;
		Dialog *dialog()				noexcept;
		int main()						noexcept;
		~Mechan()						noexcept;
	};

	extern Mechan *mechan;
}