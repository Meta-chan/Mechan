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
			Address() noexcept;
			Address(Interface::ID interface_id, unsigned long long int chat_id, unsigned long long int user_id) noexcept;
		};
		
		struct ReadResult
		{
			bool ok;
			Address address;
			std::string message;
		};
		
		virtual bool ok()															const noexcept	= 0;
		virtual Interface::ID id()													const noexcept	= 0;
		virtual bool write(const std::string message, Address address = Address())	noexcept		= 0;
		virtual ReadResult read()													noexcept		= 0;
		virtual ~Interface()														noexcept		= 0;
	};
}