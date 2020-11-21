#include "../header/parse.h"

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