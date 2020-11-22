#pragma once

#include <string>

namespace mechan
{
	class Interface
	{
	public:
		enum class ID : unsigned int
		{
			console,
			pipe,
			log,
			telegram
		};
		
		struct Address
		{
			Interface::ID interface_id;
			unsigned long long int chat_id;
			unsigned long long int user_id;
			Address();
			Address(Interface::ID interface_id, unsigned long long int chat_id, unsigned long long int user_id);
		};
		
		struct ReadResult
		{
			bool ok;
			Address address;
			std::string message;
		};
		
		virtual bool ok() const noexcept = 0;
		virtual Interface::ID id() const noexcept = 0;
		virtual bool write(const std::string message, Address address = Address()) = 0;
		virtual ReadResult read(unsigned int timeout = (unsigned int)-1) = 0;
		virtual ~Interface() = 0;
	};
}

#define MECHAN_DIR "E:\\Project\\Mechan"