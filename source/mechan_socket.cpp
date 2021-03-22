#define IR_INCLUDE 'i'
#include "../header/mechan_socket.h"
#include "../header/mechan_config.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>


mechan::TCPServer::TCPServer() noexcept
{
	ir::IP ip(Config::value("IP").c_str(), false, atoi(Config::value("PORT").c_str()));
	if (!ip.ok()) return;
	ir::TCPServer::init(ip);
}

bool mechan::TCPServer::ok() const noexcept
{
	return ir::TCPServer::ok();
}

bool mechan::TCPServer::send(TCPAddress address, const std::string message) noexcept
{
	for (size_t i = 0; i < ir::TCPServer::clinet_number(); i++)
	{
		if (address.ip == ir::TCPServer::client(i))
		{
			try { _buffer.resize(sizeof(TCPHeader) + message.size()); } catch (...) { return false; }
			TCPHeader header;
			header.chat_id = address.chat_id;
			header.user_id = address.user_id;
			header.size = message.size();
			memcpy(_buffer.data(), &header, sizeof(TCPHeader));
			memcpy(_buffer.data() + sizeof(TCPHeader), message.c_str(), message.size());
			return ir::TCPServer::send(i, _buffer.data(), _buffer.size(), 0);
		}
	}
	return false;
}

bool mechan::TCPServer::receive(TCPAddress *address, std::string *message) noexcept
{
	ir::TCPServer::accept();
	for (size_t i = 0; i < ir::TCPServer::clinet_number(); i++)
	{
		TCPHeader header;
		if (ir::TCPServer::pick(i, &header, sizeof(TCPHeader), 0))
		{
			try { _buffer.resize(sizeof(TCPHeader) + header.size); } catch (...) { return false; }
			if (ir::TCPServer::receive(i, _buffer.data(), sizeof(TCPHeader) + header.size, 0))
			{
				try { message->resize(header.size); } catch (...) { return false; }
				memcpy(&message->at(0), _buffer.data() + sizeof(TCPHeader), message->size());
				address->chat_id = header.chat_id;
				address->user_id = header.user_id;
				address->ip = ir::TCPServer::client(i);
				return true;
			}
		}
	}
	return false;
}

mechan::TCPClient::TCPClient() noexcept
{
	ir::IP ip(Config::value("IP").c_str(), false, atoi(Config::value("PORT").c_str()));
	if (!ip.ok()) return;
	if (!ir::TCPClient::init(false)) return;
	ir::TCPClient::connect(ip);
	
}

bool mechan::TCPClient::send(TCPAddress address, const std::string message) noexcept
{
	try { _buffer.resize(sizeof(TCPHeader) + message.size()); } catch (...) { return false; }
	TCPHeader header;
	header.chat_id = address.chat_id;
	header.user_id = address.user_id;
	header.size = message.size();
	memcpy(_buffer.data(), &header, sizeof(TCPHeader));
	memcpy(_buffer.data() + sizeof(TCPHeader), message.c_str(), message.size());
	return ir::TCPClient::send(_buffer.data(), _buffer.size());
}

bool mechan::TCPClient::receive(TCPAddress *address, std::string *message) noexcept
{
	TCPHeader header;
	if (ir::TCPClient::pick(&header, sizeof(TCPHeader)))
	{
		try { _buffer.resize(sizeof(TCPHeader) + header.size); } catch (...) { return false; }
		if (ir::TCPClient::receive(_buffer.data(), sizeof(TCPHeader) + header.size))
		{
			try { message->resize(header.size); }
			catch (...) { return false; }
			memcpy(&message->at(0), _buffer.data() + sizeof(TCPHeader), message->size());
			address->chat_id = header.chat_id;
			address->user_id = header.user_id;
			address->ip = ir::IP();
			return true;
		}
	}
	return false;
}

bool mechan::TCPClient::ok() const noexcept
{
	return ir::TCPClient::ok();
}