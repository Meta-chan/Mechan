#include "../header/mechan_console_interface.h"
#include <Windows.h>
#include <assert.h>

namespace mechan
{
	class PipeInterface : public Interface
	{
	private:
		HANDLE _hpipe = INVALID_HANDLE_VALUE;

	public:
		PipeInterface() noexcept;
		bool ok() const noexcept;
		Interface::ID id() const noexcept;
		bool write(const std::string message, Address address) noexcept;
		ReadResult read(unsigned int timeout) noexcept;
		~PipeInterface() noexcept;
	};
}

mechan::PipeInterface::PipeInterface()
{
	_hpipe = CreateNamedPipeW(L"\\\\.\\pipe\\mechan", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_NOWAIT, 1, 1024, 1024, 0, nullptr);
	if (_hpipe != INVALID_HANDLE_VALUE) ConnectNamedPipe(_hpipe, nullptr);
}

bool mechan::PipeInterface::ok() const noexcept
{
	return _hpipe != INVALID_HANDLE_VALUE;
}

mechan::Interface::ID mechan::PipeInterface::id() const noexcept
{
	return ID::pipe;
}

bool mechan::PipeInterface::write(const std::string message, Address address) noexcept
{
	assert(_hpipe != INVALID_HANDLE_VALUE);
	DWORD written;
	return WriteFile(_hpipe, message.c_str(), message.size() + 1, &written, nullptr);
}

mechan::Interface::ReadResult mechan::PipeInterface::read(unsigned int timeout) noexcept
{
	assert(_hpipe != INVALID_HANDLE_VALUE);
	ReadResult result;
	result.address.interface_id = ID::pipe;
	result.address.chat_id = 0;
	result.address.user_id = 0;
	result.message.resize(256);
	DWORD read;
	ReadFile(_hpipe, &result.message[0], 256, &read, nullptr);
	result.message.resize(read);
	result.ok = read != 0;
}

mechan::PipeInterface::~PipeInterface() noexcept
{
	if (_hpipe != INVALID_HANDLE_VALUE) DisconnectNamedPipe(_hpipe);
}
