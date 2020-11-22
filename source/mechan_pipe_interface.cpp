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