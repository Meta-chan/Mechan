#pragma once

#include "mechan_dialog.h"
#include "mechan_morphology.h"
#include <ir/s2st_database.h>
#include <ir/n2st_database.h>
#include <map>
#include <set>
#include <vector>
#include <stdio.h>

namespace mechan
{
	class Word
	{
	public:
		struct WordInfo
		{
			unsigned int lowercase_occurence_number;
			unsigned int uppercase_occurence_number;
			MorphologyCharacteristics probable_characteristics;
			unsigned int morphology_group_number;
			unsigned int *morphology_groups()					noexcept;		//sorted
			const unsigned int *morphology_groups()				const noexcept;	//sorted
			unsigned int synonym_group_number(unsigned int size)const noexcept;
			unsigned int *synonym_groups()						noexcept;		//sorted
			const unsigned int *synonym_groups()				const noexcept;	//sorted
		};

	private:

		struct UnpackedWordInfo
		{
			unsigned int lowercase_occurence_number = 0;
			unsigned int uppercase_occurence_number = 0;
			std::map<unsigned int, MorphologyCharacteristics> morphology_characteristics;	//sorted by group, only first characteristics for each group
			std::set<unsigned int> synonym_groups;											//sorted by group
		};

		struct UnpackedMorphologyGroupInfo
		{
			std::set<std::string> lowercase_words;	//not sorted
		};

		mechan::Dialog *_dialog					= nullptr;
		ir::S2STDatabase _words;
		std::map<std::string, UnpackedWordInfo> _unpacked_words;
		std::vector<UnpackedMorphologyGroupInfo> _unpacked_morphology_groups;
		std::vector<char> _buffer;

		//Open existing databases
		bool _load()				noexcept;
		
		//Utilities
		void _parse_morphology_add(unsigned int group, const std::string lowercase_word, MorphologyCharacteristics ch) noexcept;
		void _parse_synonym_add(unsigned int group, const std::string lowercase_word) noexcept;
		unsigned int _pack_morphology_group_occurencies(unsigned int group) const noexcept;

		//Creating databases
		bool _parse_dialog()		noexcept;
		bool _parse_morphology()	noexcept;
		bool _parse_synonym()		noexcept;
		bool _pack()				noexcept;

	public:
		Word(Dialog *dialog)		noexcept;
		bool ok()					const noexcept;
		bool word_info(std::string lowercase_word, const WordInfo **info, unsigned int *info_size) noexcept;
	};
}