#include "../header/mechan_console_interface.h"
#include <stdio.h>

namespace mechan
{
	class ConsoleInterface : public Interface
	{
	public:
		ConsoleInterface()										noexcept;
		bool ok()												const noexcept;
		Interface::ID id()										const noexcept;
		bool write(const std::string message, Address address)	noexcept;
		ReadResult read()										noexcept;
		~ConsoleInterface()										noexcept;
	};
}

mechan::Interface *mechan::new_console_interface() noexcept
{
	return new ConsoleInterface;
}

mechan::ConsoleInterface::ConsoleInterface() noexcept
{}

bool mechan::ConsoleInterface::ok() const noexcept
{
	return true;
}

mechan::Interface::ID mechan::ConsoleInterface::id() const noexcept
{
	return ID::console;
}

bool mechan::ConsoleInterface::write(const std::string message, Address address) noexcept
{
	printf("%s\n", message.c_str());
	return true;
}

mechan::Interface::ReadResult mechan::ConsoleInterface::read() noexcept
{
	//Make it non-blocking!
	ReadResult result;
	result.address.chat_id = 0;
	result.address.user_id = 0;
	result.address.interface_id = ID::console;
	result.message.reserve(128);
	result.message.resize(scanf("%128s", &result.message[0]));
	result.ok = true;
	return result;
}

mechan::ConsoleInterface::~ConsoleInterface() noexcept
{
}