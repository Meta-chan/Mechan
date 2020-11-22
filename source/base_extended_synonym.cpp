#define IR_IMPLEMENT
#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>
#include <ir_container/ir_vector.h>

//morphgroup_data format:
//word \0 DESCRIPTION \0 Wo'rd \0

struct MorphologyDataElem
{
	const char *resuced;
	unsigned int reduced_len;
	const char *full;
	unsigned int full_len;
	const char *description;
	unsigned int description_len;
};

ir::N2STDatabase *morphgroup_data = nullptr;
ir::S2STDatabase *word_syngroup = nullptr;
ir::S2STDatabase *word_exsyngroup = nullptr;

bool parse_morphology_data(ir::ConstBlock data, ir::Vector<MorphologyDataElem> result)
{
	result.resize(0);
	const char *stringdata = (const char*)data.data;
	const char *end = stringdata + data.size;
	while (stringdata < end)
	{
		//resuced
		result.resize(result.size() + 1);
		result.back().resuced = stringdata;
		while (*stringdata != '\0') stringdata++;
		result.back().reduced_len = stringdata++ - result.back().resuced;

		//description
		result.back().description = stringdata;
		while (*stringdata != '\0') stringdata++;
		result.back().description_len = stringdata++ - result.back().description;

		//full
		result.back().full = stringdata;
		while (*stringdata != '\0') stringdata++;
		result.back().full_len = stringdata++ - result.back().full;
	}
	return !result.empty();
};

bool add_syngroup(const char *word, unsigned int syngroup)
{
	ir::ConstBlock wordblock(strlen(word), word);
	ir::ConstBlock syngroups;
	ir::ec code = word_exsyngroup->read(wordblock, &syngroups);
	bool match = false;
	if (code == ir::ec::ec_ok)
	{
		for (unsigned int i = 0; i < syngroups.size / sizeof(unsigned int); i++)
		{
			if (((unsigned int*)syngroups.data)[i] == syngroup)
			{
				match = true;
				break;
			}
		}
	}

	if (!match)
	{
		syngroups.size += sizeof(unsigned int);
		unsigned int *buf = (unsigned int*)malloc(syngroups.size);
		if (buf == nullptr) return false;
		buf[syngroups.size / sizeof(unsigned int) - 1] = syngroup;
		syngroups.data = buf;
		code = word_exsyngroup->insert(wordblock, syngroups);
	}
	return true;
};

bool process_morphology_group(ir::Vector<MorphologyDataElem> group)
{
	for (unsigned int i = 0; i < group.size(); i++)
	{
		ir::ConstBlock syngroup;
		ir::ec code = word_syngroup->read(ir::ConstBlock(group[i].reduced_len, group[i].resuced), &syngroup);
		if (code == ir::ec::ec_ok)
		{
			add_syngroup(group[i].resuced, *((unsigned int*)syngroup.data));
		}
	}
	return true;
};

int _main()
{
	ir::ec code;
	morphgroup_data = new ir::N2STDatabase(L"C:\\Project\\mechan\\database\\morphgroup_data.idt", ir::Database::createmode::create_readonly, &code);
	if (code != ir::ec::ec_ok) return 1;
	word_syngroup = new ir::S2STDatabase(L"C:\\Project\\mechan\\database\\word_syngroup.idt", ir::Database::createmode::create_readonly, &code);
	if (code != ir::ec::ec_ok) return 1;
	word_exsyngroup = new ir::S2STDatabase(L"C:\\Project\\mechan\\database\\word_exsyngroup.idt", ir::Database::createmode::create_new_always, &code);
	if (code != ir::ec::ec_ok) return 1;

	for (unsigned int i = 0; i < morphgroup_data->get_table_size(); i++)
	{
		ir::ConstBlock response;
		code = morphgroup_data->read(i, &response);
		if (code != ir::ec::ec_ok) return 1;
		ir::Vector<MorphologyDataElem> morphgroup;
		parse_morphology_data(response, morphgroup);
		process_morphology_group(morphgroup);
	}
};

int main()
{
	int res = _main();
	if (morphgroup_data != nullptr) delete morphgroup_data;
	if (word_syngroup != nullptr) delete word_syngroup;
	if (word_exsyngroup != nullptr) delete word_exsyngroup;
	return res;
};