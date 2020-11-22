#include "../header/mechan_log_interface.h"

#include <stdio.h>

namespace mechan
{
	class LogInterface : public Interface
	{
	private:
		File *_file;
		
	public:
		LogInterface();
		bool ok() const noexcept;
		Interface::ID id() const noexcept;
		bool write(const std::string message, Address address);
		~LogInterface();
	} *log_interface;
}

mechan::Interface *mechan::new_log_interface()
{
	return new LogInterface;
}

mechan::Interface *mechan::log_intreface;
	
mechan::LogInterface::LogInterface()
{
	_file = fopen(MECHAN_DIR "\\log.txt", "w");
}

bool mechan::LogInterface::ok()
{
	return (_file != nullptr);
}

mechan::Interface::ID mechan::LogInterface::id()
{
	return ID::log;
}

bool mechan::LogInterface::write(const std::string message, Address address)
{
	return (fwrite(message.c_str(), message.size(), 1, _file) != 0);
}

mechan::LogInterface::~LogInterface()
{
	if (_file != nullptr) fclose(_file);
}