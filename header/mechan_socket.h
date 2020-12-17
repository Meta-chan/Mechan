#pragma once

#include <WinSock2.h>
#include <Ws2def.h>
#include <string>
#include <vector>

namespace mechan
{
	class Server
	{
	private:
		struct Header
		{
			unsigned long long int length;
			unsigned long long int chat_id;
			unsigned long long int user_id;
		};

		struct Client
		{
			SOCKET socket;
			sockaddr_in address;
		};

		bool _ok = false;
		SOCKET _socket;
		sockaddr_in _address;
		std::vector<char> _buffer;
		std::vector<Client> _clients;

	public:
		struct Address
		{
			sockaddr_in address;
			unsigned long long int chat_id;
			unsigned long long int user_id;
		};

		struct ReceiveResult
		{
			bool ok;
			std::string message;
			Address address;
		};

		Server()											noexcept;
		bool ok()											const noexcept;
		bool send(const std::string string, Address address)noexcept;
		ReceiveResult receive()								noexcept;
		~Server()											noexcept;
	};

	class Client
	{
	private:
		struct Header
		{
			unsigned long long int length;
			unsigned long long int chat_id;
			unsigned long long int user_id;
		};

		bool _ok = false;
		SOCKET _socket;
		std::vector<char> _buffer;

	public:
		struct Address
		{
			unsigned long long int chat_id;
			unsigned long long int user_id;
		};

		struct ReceiveResult
		{
			bool ok;
			std::string message;
			Address address;
		};

		Client()											noexcept;
		bool ok()											const noexcept;
		bool send(const std::string string, Address address)noexcept;
		ReceiveResult receive()								noexcept;
		~Client()											noexcept;
	};
}