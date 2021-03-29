#pragma once

#include <ir/n2st_database.h>
#include <string>

namespace mechan
{
	class Mechan;

	class Dialog
	{
	private:
		ir::N2STDatabase _dialog;
		bool _parse()								noexcept;

	public:
		Dialog()									noexcept;
		bool ok()									const noexcept;
		bool message(unsigned int i, std::string *s)noexcept;
		unsigned int count()						const noexcept;
	};
}