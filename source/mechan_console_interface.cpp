#include "../header/mechan_console_interface.h"

namespace mechan
{
	class ConsoleInterface : public Interface
	{
	public:
		ConsoleInterface();
		bool ok() const noexcept;
		Interface::ID id() const noexcept;
		bool write(const std::string message, Address address);
		ReadResult read(unsigned int timeout);
		~ConsoleInterface();
	};
}

mechan::Interface *new_console_interface()
{
	return new ConsoleInterface;
}

Interface *mechan::Interface::console_interface;

mechan::ConsoleInterface::ConsoleInterface()
{}

bool mechan::ConsoleInterface::ok() const noexcept
{
	return true;
}

Interface::ID mechan::ConsoleInterface::id() const noexcept
{
	return Interface::ID::console;
}

bool mechan::ConsoleInterface::write(const std::string message, Address address)
{
	
}

mechan::Interface::ReadResult mechan::ConsoleInterface::read(unsigned int timeout)
{
	
}

mechan::ConsoleInterface::~ConsoleInterface()
{
	
}