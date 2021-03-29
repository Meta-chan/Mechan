#define IR_INCLUDE 'i'
#include "../header/mechan_dialog.h"
#include <assert.h>
#include <ir/file.h>

bool mechan::Dialog::_parse() noexcept
{
	//Open text file
	ir::File dialog(SS("data/dialog.txt"), SS("r"));
	if (!dialog.ok()) return false;
	printf("Dialog file found\n");
	ir::uint64 dialog_size = dialog.size();

	//Open database
	if (_dialog.init(SS("data/dialog"), ir::Database::create_mode::neww) != ir::ec::ok
	|| _dialog.set_ram_mode(true, true) != ir::ec::ok)
	{
		_dialog.finalize();
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
				_dialog.insert(index, data);
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
			printf("Dialog file parsing %u", reported);
		}
	}

	_dialog.finalize();
	return true;
}

mechan::Dialog::Dialog() noexcept
{
	//First try
	if (_dialog.init(SS("data/dialog"), ir::Database::create_mode::read) == ir::ec::ok
	&& _dialog.set_ram_mode(true, true) == ir::ec::ok)
	{
		printf("Dialog database found\n");
		return;
	}
	
	//Second try
	if (_parse())
	{
		if (_dialog.init(SS("data/dialog"), ir::Database::create_mode::read) == ir::ec::ok
		&& _dialog.set_ram_mode(true, true) == ir::ec::ok)
		{
			printf("Dialog database found\n");
		}
	}
}

bool mechan::Dialog::ok() const noexcept
{
	return _dialog.ok();
}

bool mechan::Dialog::message(unsigned int i, std::string *s) noexcept
{
	assert(ok());
	if (s != nullptr)
	{
		ir::Block data;
		if (_dialog.read(i, &data) != ir::ec::ok) return false;
		s->resize(data.size());
		memcpy(&s->at(0), data.data(), data.size());
		return true;
	}
	else
	{
		return _dialog.probe(i) == ir::ec::ok;
	}
}

unsigned int mechan::Dialog::count() const noexcept
{
	assert(ok());
	return (_dialog.get_table_size());
}
