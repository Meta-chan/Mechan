#pragma once

#include <ir/types.h>
#include <ir/tcp.h>
#include <string>
#include <vector>

namespace mechan
{
	struct TCPAddress
	{
		ir::IP ip;
		ir::uint64 chat_id;
		ir::uint64 user_id;
	};

	struct TCPHeader
	{
		ir::uint64 size;
		ir::uint64 chat_id;
		ir::uint64 user_id;
	};

	class TCPServer : ir::TCPServer
	{
	private:
		std::vector<char> _buffer;

	public:
		TCPServer() noexcept;
		bool ok() const noexcept;
		bool send(TCPAddress address, const std::string message) noexcept;
		bool receive(TCPAddress *address, std::string *message) noexcept;
	};

	class TCPClient : ir::TCPClient
	{
	private:
		std::vector<char> _buffer;

	public:
		TCPClient() noexcept;
		bool ok() const noexcept; 
		bool send(TCPAddress address, const std::string message) noexcept;
		bool receive(TCPAddress *address, std::string *message) noexcept;
	};
}
