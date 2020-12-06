#include "../header/mechan_parse.h"

void parse(const std::string string, std::vector<std::string> *words) noexcept
{
	words->resize(0);
	bool request_new = true;
	for (unsigned int i = 0; i < string.size(); i++)
	{
		if((string[i] >= '0' && string[i] <= '9')
		|| (string[i] >= 'A' && string[i] <= 'z')
		|| ((unsigned char)string[i]) == 0xA8		//¨
		|| ((unsigned char)string[i]) == 0xB8		//¸
		|| ((unsigned char)string[i]) >= 0xC0)		//À - ÿ
		{
			if (request_new)
			{
				words->push_back(std::string());
				request_new = false;
			}
			words->back().push_back(string[i]);
		}
		else if (string[i] == '-')
		{
			if (!request_new) words->back().push_back(string[i]);
		}
		else request_new = true;
	}
};