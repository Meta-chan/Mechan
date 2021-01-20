#include "../header/mechan_config.h"
#include <stdio.h>

bool mechan::Config::_initialized = false;
std::map<std::string, std::string> mechan::Config::_config;

void mechan::Config::_initialize() noexcept
{
	_initialized = true;
	FILE *file = fopen("mechan_config", "r");
	if (file == nullptr) return;
	
	enum class State
	{
		endline_wait_name,
		name_wait_equal,
		space_wait_equal,
		value_wait_endline,
	} state = State::endline_wait_name;
	std::string name;
	std::string value;
	
	while(true)
	{
		char c;
		if (fread(&c, 1, 1, file) == 0)
		{
			if (!name.empty()) _config[name] = value;
			fclose(file);
			return;
		}
		else switch (state)
		{
		case State::endline_wait_name:
			if (c == '=') state = State::value_wait_endline;
			else if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {}
			else
			{
				name.push_back(c);
				state = State::name_wait_equal;
			}
			break;
			
		case State::name_wait_equal:
			if (c == '=') state = State::value_wait_endline;
			else if (c == '\n' || c == '\r')
			{
				if (!name.empty()) _config[name] = value;
				name.clear();
				value.clear();
			}
			else if (c == ' ' || c == '\t') state = State::space_wait_equal;
			else name.push_back(c);
			break;
			
		case State::space_wait_equal:
			if (c == '=') state = State::value_wait_endline;
			else if (c == '\n' || c == '\r')
			{
				if (!name.empty()) _config[name] = value;
				name.clear();
				value.clear();
			}
			else if (c == ' ' || c == '\t') {}
			else
			{
				if (!name.empty()) name.push_back(' ');
				name.push_back(c);
			}
			break;
			
		case State::value_wait_endline:
			if (c == '\n' || c == '\r')
			{
				if (!name.empty()) _config[name] = value;
				name.clear();
				value.clear();
			}
			else value.push_back(c);
			break;
		}
	}
}

bool mechan::Config::defined(const std::string name) noexcept
{
	if (!_initialized) _initialize();
	return (_config.count(name) != 0);
}

std::string mechan::Config::value(const std::string name) noexcept
{
	if (!_initialized) _initialize();
	return _config.count(name) == 0 ? "" : _config[name];
}