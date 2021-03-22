#define IR_INCLUDE 'i'
#include "../header/mechan.h"
#include <assert.h>
#include <ir/file.h>
#include <ir/print.h>

bool mechan::Dialog::_parse() noexcept
{
	//Open text file
	ir::File dialog(SS("data\\dialog.txt"), SS("r"));
	if (!dialog.ok()) return false;
	_mechan->print_event_log("Dialog file found");
	ir::uint64 dialog_size = dialog.size();

	//Open database
	_dialog = new ir::N2STDatabase(SS("data\\dialog"), ir::Database::create_mode::neww, nullptr);
	if (!_dialog->ok() || _dialog->set_ram_mode(true, true) != ir::ec::ok)
	{
		delete _dialog;
		_dialog = nullptr;
		return false;
	}
	
	//Init variables
	unsigned int reported = 0;
	unsigned int index = 0;
	std::string buffer;
	
	//Parse
	while (true)
	{
		//Read symbol
		char c;
		bool read = dialog.read(&c, 1) != 0;

		//Process symbol
		if (!read || c == '\n')
		{
			if (!buffer.empty())
			{
				ir::Block data(buffer.c_str(), buffer.size());
				_dialog->insert(index, data);
				buffer.resize(0);
			}
			index++;
			if (!read) break;
		}
		else if (c == '\r') {}
		else buffer.push_back(c);

		//Report success
		if ((unsigned int)(100.0 * dialog.tell() / dialog_size) > reported)
		{
			reported = (unsigned int)(100.0 * dialog.tell() / dialog_size);
			char buffer[64];
			ir::print(buffer, "Dialog file parsing %u", reported);
			_mechan->print_event_log(buffer);
		}
	}

	delete _dialog;
	_dialog = nullptr;
	return true;
}

mechan::Dialog::Dialog(Mechan *mechan) noexcept : _mechan(mechan)
{
	//First try
	_dialog = new ir::N2STDatabase(SS("data\\dialog"), ir::Database::create_mode::read, nullptr);
	if (_dialog->ok() && _dialog->set_ram_mode(true, true) == ir::ec::ok)
	{
		_mechan->print_event_log("Dialog database found");
		return;
	}
	
	//Second try
	if (_parse())
	{
		_dialog = new ir::N2STDatabase(SS("data\\dialog"), ir::Database::create_mode::read, nullptr);
		if (_dialog->ok() && _dialog->set_ram_mode(true, true) == ir::ec::ok)
		{
			_mechan->print_event_log("Dialog database found");
		}
	}
}

bool mechan::Dialog::ok() const noexcept
{
	return _dialog != nullptr && _dialog->ok();
}

bool mechan::Dialog::message(unsigned int i, std::string *s) const noexcept
{
	assert(ok());
	if (s != nullptr)
	{
		ir::Block data;
		if (_dialog->read(i, &data) != ir::ec::ok) return false;
		s->resize(data.size());
		memcpy(&s->at(0), data.data(), data.size());
		return true;
	}
	else
	{
		return _dialog->probe(i) == ir::ec::ok;
	}
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
