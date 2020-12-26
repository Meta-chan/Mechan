#include "../header/mechan_socket.h"
#include <ws2tcpip.h>
#include <assert.h>

#define SERVER_IP "192.168.0.4"
#define SERVER_PORT 30000

mechan::Server::Server() noexcept
{
	//Startup
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) return;

	//Create socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET) return;

	//Bind socket
	sockaddr_in address;
	memset(&address, 0, sizeof(sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_IP, &address.sin_addr) != 1) return;
	if (bind(_socket, (sockaddr*)&address, sizeof(sockaddr_in)) == SOCKET_ERROR) return;
	
	//Listening socket
	if (listen(_socket, 5) != 0) return;

	//Set non-blocking mode
	unsigned long nonblocking = 1;
	if (ioctlsocket(_socket, FIONBIO, &nonblocking) == SOCKET_ERROR) return;

	_ok = true;
}

bool mechan::Server::ok() const noexcept
{
	return _ok;
}

bool mechan::Server::send(std::string string, Address address) noexcept
{
	assert(_ok);

	for (unsigned int i = 0; i < _clients.size(); i++)
	{
		if (memcmp(&_clients[i].address, &address.address, sizeof(sockaddr_in)) == 0)
		{
			_buffer.resize(sizeof(Header) + string.size());
			Header header;
			header.chat_id = address.chat_id;
			header.user_id = address.user_id;
			header.length = string.size();
			memcpy(&_buffer[0], &header, sizeof(Header));
			memcpy(&_buffer[sizeof(Header)], string.data(), string.size());
			if (::send(_clients[i].socket, _buffer.data(), (int)_buffer.size(), 0) == SOCKET_ERROR)
			{
				closesocket(_clients[i].socket);
				_clients.erase(_clients.begin() + i);
				return false;
			}
		}
	}
	return false;
}

mechan::Server::ReceiveResult mechan::Server::receive() noexcept
{
	assert(_ok);
	Client newclient;
	int newaddress_size = sizeof(sockaddr_in);
	newclient.socket = ::accept(_socket, (sockaddr*)&newclient.address, &newaddress_size);
	if (newclient.socket != INVALID_SOCKET) _clients.push_back(newclient);

	for (unsigned int i = 0; i < _clients.size(); i++)
	{
		Header header;
		int received = ::recv(_clients[i].socket, (char*)&header, sizeof(Header), MSG_PEEK);
		if (received != SOCKET_ERROR && received >= sizeof(Header))
		{
			unsigned long available;
			ioctlsocket(_clients[i].socket, FIONREAD, &available);
			if (available >= sizeof(Header) + header.length)
			{
				ReceiveResult result;
				result.ok = true;
				result.address.address = _clients[i].address;
				result.address.chat_id = header.chat_id;
				result.address.user_id = header.user_id;
				result.message.resize((size_t)header.length);
				::recv(_clients[i].socket, (char*)&header, sizeof(Header), 0);
				::recv(_clients[i].socket, &result.message[0], (int)header.length, 0);
				return result;
			}
		}
	}
	ReceiveResult result;
	result.ok = false;
	return result;
}

mechan::Server::~Server() noexcept
{
	for (unsigned int i = 0; i < _clients.size(); i++)
	{
		closesocket(_clients[i].socket);
	}
	closesocket(_socket);
	_ok = false;
}

mechan::Client::Client() noexcept
{
	//Startup
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) return;

	//Create socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET) return;

	//Set non-blocking mode
	unsigned long nonblocking = 1;
	if (ioctlsocket(_socket, FIONBIO, &nonblocking) == SOCKET_ERROR) return;

	//Start connection
	sockaddr_in address;
	memset(&address, 0, sizeof(sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_IP, &address.sin_addr) != 1) return;
	::connect(_socket, (sockaddr*)&address, sizeof(sockaddr_in));

	//Wait for conenction
	fd_set write_set;
	FD_ZERO(&write_set);
	FD_SET(_socket, &write_set);
	TIMEVAL timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	if (select(0, nullptr, &write_set, nullptr, &timeout) != 1) return;
	
	_ok = true;
}

bool mechan::Client::ok() const noexcept
{
	return _ok;
}

bool mechan::Client::send(std::string string, Address address) noexcept
{
	assert(_ok);

	_buffer.resize(sizeof(Header) + string.size());
	Header header;
	header.chat_id = address.chat_id;
	header.user_id = address.user_id;
	header.length = string.size();
	memcpy(&_buffer[0], &header, sizeof(Header));
	memcpy(&_buffer[sizeof(Header)], string.data(), string.size());
	if (::send(_socket, _buffer.data(), (int)_buffer.size(), 0) == SOCKET_ERROR)
	{
		_ok = false;
		return false;
	}

	return true;
}

mechan::Client::ReceiveResult mechan::Client::receive() noexcept
{
	assert(_ok);
	Client newclient;
	int newaddress_size = sizeof(sockaddr_in);
	
	Header header;
	int received = ::recv(_socket, (char*)&header, sizeof(Header), MSG_PEEK);
	if (received != SOCKET_ERROR && received >= sizeof(Header))
	{
		unsigned long available;
		ioctlsocket(_socket, FIONREAD, &available);
		if (available >= sizeof(Header) + header.length)
		{
			ReceiveResult result;
			result.ok = true;
			result.address.chat_id = header.chat_id;
			result.address.user_id = header.user_id;
			result.message.resize((size_t)header.length);
			::recv(_socket, (char*)&header, sizeof(Header), 0);
			::recv(_socket, &result.message[0], (int)header.length, 0);
			return result;
		}
	}
	ReceiveResult result;
	result.ok = false;
	return result;
}

mechan::Client::~Client() noexcept
{
	closesocket(_socket);
	_ok = false;
}