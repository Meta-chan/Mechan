#include "../header/mechan_dialog.h"
#include "../header/mechan_directory.h"
#include <assert.h>
#include <stdio.h>

namespace mechan
{
	class Parser
	{
	private:
		FILE *_text					= nullptr;
		ir::N2STDatabase *_dialog	= nullptr;

	public:
		bool parse();
		~Parser();
	};
}

bool mechan::Parser::parse()
{
	_text = fopen(MECHAN_DIR, "r");
	if (_text == nullptr) return false;
	_dialog = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\dialog", ir::Database::create_mode::neww, nullptr);
	if (!_dialog->ok()) return false;

	unsigned int i = 0;
	std::string buffer;
	while (true)
	{
		char c;
		if (fread(&c, 1, 1, _text) == 0)
		{
			if (!buffer.empty())
			{
				ir::ConstBlock data(buffer.size() + 1, buffer.c_str());
				_dialog->insert(i, data);
				break;
			}
		}
		else if (c == '\n')
		{
			ir::ConstBlock data(buffer.size() + 1, buffer.c_str());
			_dialog->insert(i, data);
			buffer.resize(0);
			i++;
		}
		else if (c == '\r') {}
		else buffer.push_back(c);
	}
	return true;
}

mechan::Parser::~Parser()
{
	if (_dialog != nullptr) delete _dialog;
	if (_text != nullptr) fclose(_text);
}

mechan::Dialog::Dialog() noexcept
{
	//First try
	_dialog = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\dialog", ir::Database::create_mode::read, nullptr);
	if (_dialog->ok()) return;
	
	//Second try
	delete _dialog;
	bool parsed = false;
	{
		Parser parser;
		parsed = parser.parse();
	}
	if (parsed) _dialog = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\dialog", ir::Database::create_mode::read, nullptr);
	else _dialog = nullptr;
}

bool mechan::Dialog::ok() const noexcept
{
	return _dialog != nullptr && _dialog->ok();
}

std::string mechan::Dialog::dialog(unsigned int i) const noexcept
{
	ir::ConstBlock data;
	if (_dialog->read(i, &data) == ir::ec::ok) return std::string((const char*)data.data, data.size - 1);
	else return std::string();
}

unsigned int mechan::Dialog::count() const noexcept
{
	return (_dialog->count());
}

mechan::Dialog::~Dialog() noexcept
{
	if (_dialog != nullptr) delete _dialog;
}