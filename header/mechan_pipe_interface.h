#pragma once

#include "mechan_interface.h"

namespace mechan
{
	class PipeInterface : public Interface
	{
	public:
		PipeInterface();
		bool ok() const noexcept;
		Interface::ID id() const noexcept;
		bool write(const std::string message);
		ReadResult read(unsigned int timeout);
		~PipeInterface();
	} *pipe_interface;
}