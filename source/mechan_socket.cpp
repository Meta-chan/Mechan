#include "../header/mechan_socket.h"
#include "../header/mechan_config.h"

#ifdef _WIN32
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/fcntl.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	namespace mechan
	{
		static const int INVALID_SOCKET = -1;
		static const int SOCKET_ERROR = -1;
	}
#endif
#include <string.h>
#include <stdlib.h>
#include <assert.h>

mechan::Server::Server() noexcept
{
	#ifdef _WIN32
		//Startup
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) return;
	#endif

	//Create socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET) return;

	//Bind socket
	sockaddr_in address;
	memset(&address, 0, sizeof(sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(Config::value("PORT").data()));
	if (inet_pton(AF_INET, Config::value("IP").data(), &address.sin_addr) != 1) return;
	if (bind(_socket, (sockaddr*)&address, sizeof(sockaddr_in)) == SOCKET_ERROR) return;
	
	//Listening socket
	if (listen(_socket, 5) != 0) return;

	//Set non-blocking mode
	#ifdef _WIN32
		unsigned long nonblocking = 1;
		if (ioctlsocket(_socket, FIONBIO, &nonblocking) == SOCKET_ERROR) return;
	#else
		int options = fcntl(_socket, F_GETFL, NULL);
		if (options < 0) return;
		if (fcntl(_socket, F_SETFL, options | O_NONBLOCK) < 0) return;
	#endif
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
				#ifdef _WIN32
					closesocket(_clients[i].socket);
				#else
					close(_clients[i].socket);
				#endif
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
	#ifdef _WIN32
		int newaddress_size = sizeof(sockaddr_in);
	#else
		socklen_t newaddress_size = sizeof(sockaddr_in);
	#endif
	newclient.socket = ::accept(_socket, (sockaddr*)&newclient.address, &newaddress_size);
	if (newclient.socket != INVALID_SOCKET) _clients.push_back(newclient);

	for (unsigned int i = 0; i < _clients.size(); i++)
	{
		Header header;
		int received = ::recv(_clients[i].socket, (char*)&header, sizeof(Header), MSG_PEEK);
		if (received != SOCKET_ERROR && received >= sizeof(Header))
		{
			unsigned long available;
			#ifdef _WIN32
				ioctlsocket(_clients[i].socket, FIONREAD, &available);
			#else
				ioctl(_clients[i].socket, FIONREAD, &available);
			#endif
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
		#ifdef _WIN32
			closesocket(_clients[i].socket);
		#else
			close(_clients[i].socket);
		#endif
	}
	#ifdef _WIN32
		closesocket(_socket);
	#else
		close(_socket);
	#endif
	_ok = false;
}

mechan::Client::Client() noexcept
{
	#ifdef _WIN32
		//Startup
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) return;
	#endif
	
	//Create socket
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET) return;

	//Set non-blocking mode
	#ifdef _WIN32
		unsigned long nonblocking = 1;
		if (ioctlsocket(_socket, FIONBIO, &nonblocking) == SOCKET_ERROR) return;
	#else
		int options = fcntl(_socket, F_GETFL, NULL);
		if (options < 0) return;
		if (fcntl(_socket, F_SETFL, options | O_NONBLOCK) < 0) return;
	#endif
	//Start connection
	sockaddr_in address;
	memset(&address, 0, sizeof(sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(Config::value("PORT").data()));
	if (inet_pton(AF_INET, Config::value("IP").data(), &address.sin_addr) != 1) return;
	::connect(_socket, (sockaddr*)&address, sizeof(sockaddr_in));

	//Wait for conenction
	fd_set write_set;
	FD_ZERO(&write_set);
	FD_SET(_socket, &write_set);
	timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	if (select(_socket + 1, nullptr, &write_set, nullptr, &timeout) != 1) return;	
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
		#ifdef _WIN32
			ioctlsocket(_socket, FIONREAD, &available);
		#else
			ioctl(_socket, FIONREAD, &available);
		#endif
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
	#ifdef _WIN32
		closesocket(_socket);
	#else
		close(_socket);
	#endif
	_ok = false;
}
