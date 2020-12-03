#pragma once

#include <ir_database/ir_n2st_database.h>
#include <string>

namespace mechan
{
	class Dialog
	{
	private:
		ir::N2STDatabase *_dialog			= nullptr;

	public:
		Dialog()							noexcept;
		bool ok()							const noexcept;
		std::string dialog(unsigned int i)	const noexcept;
		unsigned int count()				const noexcept;
		~Dialog()							noexcept;
	};
}