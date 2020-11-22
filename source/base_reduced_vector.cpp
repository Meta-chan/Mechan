#include <string>
#include <vector>

#define IR_IMPLEMENT
#include <ir_database/ir_n2st_database.h>
#include <ir_database/ir_s2st_database.h>

ir::N2STDatabase *dialog;
ir::S2STDatabase *invectors, *outvectors;
#define MECHAN_DATABASE_DIR L"C:\\Project\\_source\\Mechan"

bool init()
{
	ir::ec code;
	dialog = new ir::N2STDatabase(MECHAN_DATABASE_DIR "/dialog.idt", ir::Database::create_readonly, &code);
	if (code != ir::ec::ec_ok) return false;
	invectors = new ir::S2STDatabase(MECHAN_DATABASE_DIR "/wikivectors.idt", ir::Database::create_readonly, &code);
	if (code != ir::ec::ec_ok) return false;
	outvectors = new ir::S2STDatabase(MECHAN_DATABASE_DIR "/commonvectors.idt", ir::Database::create_new_always, &code);
	if (code != ir::ec::ec_ok) return false;
	return true;
};

void parse(const std::string s, std::vector<std::string> *words)
{
	words->resize(0);
	bool requestnew = true;
	unsigned int i = 0;
	while (true)
	{
		if (i == s.size() || s[i] == '\0')
		{
			return;
		}
		else if ((s[i] >= '0' && s[i] <= '9')
			|| (s[i] >= 'A' && s[i] <= 'z')
			|| ((unsigned char)s[i]) == 0xA8
			|| ((unsigned char)s[i]) == 0xB8
			|| ((unsigned char)s[i]) >= 0xC0)
		{
			if (requestnew)
			{
				words->push_back("");
				requestnew = false;
			}
			words->at(words->size() - 1).push_back(s[i]);
		}
		else if (s[i] == '-')
		{
			if (!requestnew) words->at(words->size() - 1).push_back(s[i]);
		}
		else
		{
			requestnew = true;
		}
		i++;
	};
};

void process()
{
	for (int i = 0; i < dialog->get_table_size(); i++)
	{
		ir::ConstBlock reply;
		ir::ec code = dialog->read(i, &reply);
		if (code != ir::ec::ec_ok) continue;
		
		//Read
		std::string s(reply.size, '\0');
		memcpy(&s[0], reply.data, reply.size);
		
		//Parse
		std::vector<std::string> result;
		parse(s, &result);

		//For each word
		for (unsigned int j = 0; j < result.size(); j++)
		{
			ir::ConstBlock key(result[j].size(), result[j].c_str());
			ir::ConstBlock data;
			code = invectors->read(key, &data);
			if (code != ir::ec::ec_ok) continue;
			outvectors->insert(key, data, ir::Database::insertmode::insert_not_existing);
		}
	}
};

void finalize()
{
	if (dialog != nullptr) delete dialog;
	if (invectors != nullptr) delete invectors;
	if (outvectors != nullptr) delete outvectors;
};

int main()
{
	if (init()) process();
	finalize();
};