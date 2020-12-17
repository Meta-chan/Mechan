#pragma once

#include <ir_database/ir_n2st_database.h>
#include <string>

namespace mechan
{
	class Mechan;

	class Dialog
	{
	private:
		Mechan *_mechan						= nullptr;
		ir::N2STDatabase *_dialog			= nullptr;

	public:
		Dialog(Mechan *mechan)				noexcept;
		bool ok()							const noexcept;
		std::string dialog(unsigned int i)	const noexcept;
		unsigned int count()				const noexcept;
		~Dialog()							noexcept;
	};
}