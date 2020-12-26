#include "../header/mechan.h"
#include "../header/mechan_directory.h"
#include <assert.h>
#include <stdio.h>

namespace mechan
{
	class DialogParser
	{
	private:
		Mechan *_mechan				= nullptr;
		FILE *_text					= nullptr;
		ir::N2STDatabase *_dialog	= nullptr;

	public:
		DialogParser(Mechan *mechan)	noexcept;	
		bool parse()					noexcept;
		~DialogParser()					noexcept;
	};
}

mechan::DialogParser::DialogParser(Mechan *mechan) noexcept : _mechan(mechan)
{}

bool mechan::DialogParser::parse() noexcept
{
	_dialog = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\dialog", ir::Database::create_mode::neww, nullptr);
	if (!_dialog->ok()) return false;
	_dialog->set_ram_mode(true, true);
	_text = fopen(MECHAN_DIR "\\data\\dialog.txt", "r");
	if (_text == nullptr) return false;
	_mechan->print_event_log("Dialog file found");

	unsigned int i = 0;
	std::string buffer;
	while (true)
	{
		char c;
		bool read = fread(&c, 1, 1, _text) != 0;
		if (!read || c == '\n')
		{
			if (!buffer.empty())
			{
				ir::ConstBlock data(buffer.c_str(), buffer.size());
				_dialog->insert(i, data);
				buffer.resize(0);
			}
			i++;
			if (!read) break;
		}
		else if (c == '\r') {}
		else buffer.push_back(c);
	}
	return true;
}

mechan::DialogParser::~DialogParser() noexcept
{
	if (_dialog != nullptr) delete _dialog;
	if (_text != nullptr) fclose(_text);
}

mechan::Dialog::Dialog(Mechan *mechan) noexcept : _mechan(mechan)
{
	//First try
	_dialog = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\dialog", ir::Database::create_mode::read, nullptr);
	if (_dialog->ok())
	{
		_mechan->print_event_log("Dialog database found");
		_dialog->set_ram_mode(true, true); return;
	}
	
	//Second try
	delete _dialog;
	bool parsed = false;
	{
		DialogParser parser(mechan);
		parsed = parser.parse();
	}
	if (parsed)
	{
		_dialog = new ir::N2STDatabase(WIDE_MECHAN_DIR "\\data\\dialog", ir::Database::create_mode::read, nullptr);
		if (_dialog->ok())
		{
			_mechan->print_event_log("Dialog database found");
			_dialog->set_ram_mode(true, true);
		}
	}
	else _dialog = nullptr;
}

bool mechan::Dialog::ok() const noexcept
{
	return _dialog != nullptr && _dialog->ok();
}

std::string mechan::Dialog::dialog(unsigned int i) const noexcept
{
	assert(ok());
	ir::ConstBlock data;
	if (_dialog->read(i, &data) == ir::ec::ok) return std::string((const char*)data.data(), data.size());
	else return std::string();
}

unsigned int mechan::Dialog::count() const noexcept
{
	assert(ok());
	return (_dialog->get_table_size());
}

mechan::Dialog::~Dialog() noexcept
{
	if (_dialog != nullptr) delete _dialog;
}