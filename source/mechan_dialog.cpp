#include "../header/mechan.h"
#include <assert.h>
#include <stdio.h>
#include <ir_resource/ir_file_resource.h>

bool mechan::Dialog::_parse() noexcept
{
	//Open text file
	ir::FileResource dialog = fopen("data\\dialog.txt", "r");
	if (dialog == nullptr) return false;
	_mechan->print_event_log("Dialog file found");
	fseek(dialog, 0, SEEK_END);
	unsigned int dialog_size = ftell(dialog);
	fseek(dialog, 0, SEEK_SET);

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
	unsigned int position = 0;
	unsigned int index = 0;
	std::string buffer;
	
	//Parse
	while (true)
	{
		//Read symbol
		char c;
		bool read = fread(&c, 1, 1, dialog) != 0;

		//Process symbol
		if (!read || c == '\n')
		{
			if (!buffer.empty())
			{
				ir::ConstBlock data(buffer.c_str(), buffer.size());
				_dialog->insert(index, data);
				buffer.resize(0);
			}
			index++;
			if (!read) break;
		}
		else if (c == '\r') {}
		else buffer.push_back(c);

		//Report success
		position++;
		if ((unsigned int)(100.0 * position / dialog_size) > reported)
		{
			reported = (unsigned int)(100.0 * position / dialog_size);
			char buffer[64];
			sprintf(buffer, "Dialog file parsing %u", reported);
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
		ir::ConstBlock data;
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