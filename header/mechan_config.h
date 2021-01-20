#pragma once

#include <map>
#include <string>

namespace mechan
{
	class Config
	{
	private:
		static bool _initialized;
		static std::map<std::string, std::string> _config;
		static void _initialize() noexcept;
		
	public:
		static bool defined(const std::string name)			noexcept;
		static std::string value(const std::string name)	noexcept;
	};
}