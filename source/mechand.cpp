Mechan::Mechan(Callback *callback, void *user, bool *ok)
{
	ir::ec code;
	_dialog = new ir::N2STDatabase(MECHAN_DATABASE_DIR "/dialog.idt", ir::Database::create_readonly, &code);
	if (code != ir::ec::ec_ok) return;
	_dialog->set_ram_mode(true, true);
	//_vectors = new ir::S2STDatabase(MECHAN_DATABASE_DIR "/commonvectors.idt", ir::Database::create_readonly, &code);
	if (code != ir::ec::ec_ok) return;
	//_vectors->set_ram_mode(true, true);
	_callback = callback;
	_user = user;
	_ok = true;
	*ok = true;
};

void Mechan::_parse(const std::string s, std::vector<std::string> *words)
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
		|| ((unsigned char) s[i]) == 0xA8
		|| ((unsigned char) s[i]) == 0xB8
		|| ((unsigned char) s[i]) >= 0xC0)		
		{
			char c;
			if (s[i] >= 'A' && s[i] <= 'Z') c = s[i] + ('a' - 'A');
			else if (((unsigned char) s[i]) == 0xA8 || ((unsigned char) s[i]) == 0xB8) c = (char)0xE5;
			else if (((unsigned char) s[i]) >= 0xC0 && ((unsigned char) s[i]) < 0xE0) c = (char)((unsigned char)s[i] + 0x20);
			else c = s[i];
			
			if (requestnew)
			{
				words->push_back("");
				requestnew = false;
			}
			words->at(words->size() - 1).push_back(c);
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

float Mechan::_multiply(const Vector *v1, const Vector *v2)
{
	if (v1->squaresum != 0.0f && v2->squaresum != 0.0f)
	{
		float sum = 0.0f;
		for (unsigned int i = 0; i < v1->vector.size(); i++)
		{
			sum += v1->vector[i] * v2->vector[i];
		}
		return sum / sqrtf(v1->squaresum * v2->squaresum);
	}
	else return 0.0f;
};

void Mechan::_readvector(const std::string s, Vector *v)
{
	ir::ConstBlock key(s.size(), s.c_str());
	ir::ConstBlock data;
	ir::ec code = _vectors->read(key, &data);
	if (code == ir::ec::ec_ok)
	{
		v->vector.resize(data.size / sizeof(float));
		memcpy(&v->vector[0], data.data, data.size);
		float sum = 0.0f;
		for (unsigned int i = 0; i < v->vector.size(); i++)
		{
			sum += v->vector[i] * v->vector[i]; 
		}
		v->squaresum = sum;
	}
	else v->squaresum = 0.0f;;
};

float Mechan::_heuristic(const std::string s, const std::vector<std::string> *messagewords, const std::vector<Vector> *messagevectors)
{
	//Parse message
	std::vector<std::string> words;
	_parse(s, &words);
	
	//For all vectors
	float sum = 0.0f;
	for (unsigned int i = 0; i < words.size(); i++)
	{
		Vector wordvector;
		_readvector(words[i], &wordvector);
		float h = _multiply(&wordvector, &messagevectors->at(0));
		for (unsigned int j = 1; j < messagevectors->size(); j++)
		{
			float nh = _multiply(&wordvector, &messagevectors->at(j));
			if (nh > h) h = nh;
		}
		sum += h;
	}
	return sum / messagewords->size();
};

void Mechan::process(std::int64_t chatid, std::string message)
{
	std::string message1251 = (const char*)utf_buffer_recode(&utf_utf8, message.c_str(), ' ', &utf_1251);
	
	//Parse message
	std::vector<std::string> messagewords;
	_parse(message1251, &messagewords);

	//Read vectors
	std::vector<Vector> messagevectors;
	messagevectors.resize(messagewords.size());
	for (unsigned int i = 0; i < messagewords.size(); i++)
	{
		_readvector(messagewords[i], &messagevectors[i]);
	}

	float bestheuristic = 0.0f;
	std::string bestreply;
	bool nextnonzero = false;
	unsigned int reported = 0;
	for (int i = /*_dialog->tablesize() - 1*/2000; i >= 0; i--)
	{
		/*
		unsigned int progress = 100.0f * (_dialog->tablesize() - i) / _dialog->tablesize();
		if (progress > reported)
		{
			printf("%u%%\n", progress);
			reported = progress;
		}
		*/

		//If message is not empty
		ir::ConstBlock reply;
		ir::ec code = _dialog->read(i, &reply);
		if (code == ir::ec::ec_ok && reply.size != 0)
		{
			//If next message was not empty
			if (nextnonzero)
			{
				//Do heuristic
				std::string candidate(reply.size, '\0');
				memcpy(&candidate[0], reply.data, reply.size);
				float h = _heuristic(candidate, &messagewords, &messagevectors);
				if (h > bestheuristic)
				{
					//Compare with best
					code = _dialog->read(i + 1, &reply);
					if (code == ir::ec::ec_ok)
					{
						bestheuristic = h;
						bestreply.resize(reply.size);
						memcpy(&bestreply[0], reply.data, reply.size);
					}
				}
			}
			nextnonzero = true;
		}
		else nextnonzero = false;
	}
	
	_callback(_user, chatid, (const char*)utf_buffer_recode(&utf_1251, bestreply.c_str(), ' ', &utf_utf8));
};

Mechan::~Mechan()
{
	if (_dialog != nullptr) delete _dialog;
	if (_vectors != nullptr) delete _vectors;
};
