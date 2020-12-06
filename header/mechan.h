#pragma once

namespace mechan
{
	class Interface;
	class Dialog;
	class Morphology;
	class Synonym;
	class Neuro;
	class Core;

	class Mechan
	{
	private:
		Interface *_console_interface	= nullptr;
		Interface *_pipe_interface		= nullptr;
		Interface *_log_interface		= nullptr;
		Interface *_telegram_interface	= nullptr;
		Dialog *_dialog					= nullptr;
		Morphology *_morphology			= nullptr;
		Synonym *_synonym				= nullptr;
		Neuro *_neuro					= nullptr;
		Core *_core						= nullptr;

	public:
		Mechan()						noexcept;
		Interface *console_interface()	noexcept;
		Interface *pipe_interface()		noexcept;
		Interface *log_interface()		noexcept;
		Interface *telegram_interface()	noexcept;
		Dialog *dialog()				noexcept;
		Morphology *morphology()		noexcept;
		Synonym *synonym()				noexcept;
		Neuro *neuro()					noexcept;
		int main()						noexcept;
		~Mechan()						noexcept;
	};

	extern Mechan *mechan;
}