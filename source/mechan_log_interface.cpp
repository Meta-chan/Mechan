#include "../header/mechan_log_interface.h"
#include "../header/mechan_directory.h"

#include <stdio.h>

namespace mechan
{
	class LogInterface : public Interface
	{
	private:
		FILE *_file;
		
	public:
		LogInterface()											noexcept;
		bool ok()												const noexcept;
		Interface::ID id()										const noexcept;
		bool write(const std::string message, Address address)	noexcept;
		ReadResult read()										noexcept;
		~LogInterface()											noexcept;
	};
}

mechan::Interface *mechan::new_log_interface() noexcept
{
	return new LogInterface;
}
	
mechan::LogInterface::LogInterface() noexcept
{
	_file = _wfopen(WIDE_MECHAN_DIR "\\log.txt", L"w");
}

bool mechan::LogInterface::ok() const noexcept
{
	return (_file != nullptr);
}

mechan::Interface::ID mechan::LogInterface::id() const noexcept
{
	return ID::log;
}

bool mechan::LogInterface::write(const std::string message, Address address) noexcept
{
	return (fwrite(message.c_str(), message.size(), 1, _file) != 0);
}

mechan::Interface::ReadResult mechan::LogInterface::read() noexcept
{
	ReadResult result;
	result.ok = false;
	result.address.interface_id = ID::log;
	return result;
}

mechan::LogInterface::~LogInterface()
{
	if (_file != nullptr) fclose(_file);
}