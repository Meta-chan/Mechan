#define IR_IMPLEMENT
#include <string>
#include <vector>
#include <string.h>
#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>

#include "header/lowercase.h"
#include "header/parse.h"

ir::N2STDatabase *dialog;
ir::S2STDatabase *stats;

struct StatsElem
{
	unsigned int count;
	unsigned int uppercount;
	unsigned int yocount;
};

void count(std::string s)
{
	bool uppercase = false;
	bool yo = false;
	lowercase(&s, &uppercase, &yo);
	ir::ConstBlock data;
	ir::ec code = stats->read(ir::ConstBlock(s.size(), s.data()), &data);
	
	StatsElem elem;
	if (code == ir::ec::ec_ok)
	{
		memcpy(&elem, data.data, sizeof(StatsElem));
	}
	else
	{
		elem.count = 0;
		elem.uppercount = 0;
		elem.yocount = 0;
	}

	elem.count++;
	if (uppercase) elem.uppercount++;
	if (yo) elem.yocount++;
	stats->insert(ir::ConstBlock(s.size(), s.data()), ir::ConstBlock(sizeof(StatsElem), &elem));
};

int main()
{
	ir::ec code;
	dialog = new ir::N2STDatabase(L"C:\\Project\\mechan\\database\\dialog.idt", ir::Database::createmode::create_readonly, &code);
	if (code != ir::ec::ec_ok) return 1;
	stats = new ir::S2STDatabase(L"C:\\Project\\mechan\\database\\word2stat.idt", ir::Database::createmode::create_new_always, &code);
	if (code != ir::ec::ec_ok) return 2;

	for (unsigned int i = 0; i < dialog->get_table_size(); i++)
	{
		ir::ConstBlock reply;
		ir::ec code = dialog->read(i, &reply);
		if (code == ir::ec::ec_ok && reply.size != 0)
		{
			std::string s(reply.size, '\0');
			memcpy(&s[0], reply.data, reply.size);
			std::vector<std::string> result;
			parse(s, &result);
			for (unsigned int j = 0; j < result.size(); j++)
			{
				count(result[j]);
			}
		}
	}

	delete dialog;
	delete stats;
};