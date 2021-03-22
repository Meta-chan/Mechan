#define IR_INCLUDE 'a'
#include <ir/tcp.h>
#include <ir/n2st_database.h>
#include <ir/s2st_database.h>
#include <ir/neuro.h>
#include <ir/file.h>
#include <ir/print.h>
#include <ir/encoding.h>

template class ir::Neuro<double, 1>;
template class ir::Resource<void*>;
template size_t ir::Encoding::recode<ir::Encoding::UTF8, ir::Encoding::UTF16>(
	ir::Encoding::UTF16::Char*,
	const ir::Encoding::UTF8::Char*,
	const ir::Encoding::UTF16::Char*) noexcept;
template size_t ir::Encoding::recode<ir::Encoding::UTF8, ir::Encoding::CP1251>(
	ir::Encoding::CP1251::Char*,
	const ir::Encoding::UTF8::Char*,
	const ir::Encoding::CP1251::Char*) noexcept;
template size_t ir::Encoding::recode<ir::Encoding::CP866, ir::Encoding::CP1251>(
	ir::Encoding::CP1251::Char*,
	const ir::Encoding::CP866::Char*,
	const ir::Encoding::CP1251::Char*) noexcept;
template size_t ir::Encoding::recode<ir::Encoding::CP1251, ir::Encoding::CP866>(
	ir::Encoding::CP866::Char*,
	const ir::Encoding::CP1251::Char*,
	const ir::Encoding::CP866::Char*) noexcept;