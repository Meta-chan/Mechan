#pragma once

#include <ir_database/ir_s2st_database.h>
#include <vector>

namespace mechan
{
	class Mechan;

	class Synonym
	{
	private:
		Mechan *_mechan;
		ir::S2STDatabase *_word2exsyngroup = nullptr;

	public:
		Synonym(Mechan *mechan)																	noexcept;
		bool ok()																				const noexcept;
		void extended_syngroup(std::string lowercase_word, std::vector<unsigned int> *groups)	const noexcept;
		~Synonym()																				noexcept;
	};
}