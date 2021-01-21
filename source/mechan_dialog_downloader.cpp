#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#include <windows.h>
	#include <winhttp.h>
	#pragma comment(lib, "winhttp.lib")
#endif

#include "../header/mechan_character.h"

#define IR_IMPLEMENT
#include <ir_codec.h>
#include <ir_container/ir_quiet_vector.h>
#ifdef _WIN32
	#include <ir_resource/ir_hinternet_resource.h>
#endif

#include <vector>
#include <string>
#include <stdio.h>

namespace mechan
{
	class DialogDownloader
	{
	public:
		enum class Resource
		{
			ficbook_net,
			fanfics_me
		};

	private:
		enum class State
		{
			plain_text_wait_line,
			tag_wait_close,
			plain_text_wait_dash,
			direct_speech_wait_dot,
			direct_speech_wait_dash,
			author_speech_wait_dot,
			author_speech_wait_dash,
			corrupted_wait_line
		};

		static const size_t max_direct_speech = 300;
		static const size_t max_author_speech = 300;
		static const size_t max_plain_text = 300;
		

		ir::QuietVector<char> _page_utf8;
		ir::QuietVector<char> _page_cp1251;
		std::vector<std::string> _dialog;
		FILE *_file = nullptr;
		Resource _resource;

		bool _request_new_dialog = true;
		bool _request_new_message = true;
		State _state = State::plain_text_wait_line;
		size_t _direct_count = 0;
		size_t _author_count = 0;
		size_t _plain_count = 0;
		#ifdef _WIN32
			HINTERNET _session = NULL;
			HINTERNET _connection = NULL;
		#endif

		static bool _is_edge_symbol(char c);
		static bool _is_repeated_symbol(char c);
		void _push_char(char c);
		void _plain_text_wait_line(char c);
		void _plain_text_wait_dash(char c);
		void _direct_speech_wait_dot(char c);
		void _direct_speech_wait_dash(char c);
		void _author_speech_wait_dot(char c);
		void _author_speech_wait_dash(char c);
		void _corrupred_wait_line(char c);
		void _tad_wait_close(char c);
		void _parse();
		bool _download_page(const char *object);
		bool _contains(const char *word, bool exact = false);
		bool _filter_head();
		bool _filter_body();
		void _write();

	public:
		DialogDownloader(Resource resource);
		void download();
		~DialogDownloader();
	};
}

bool mechan::DialogDownloader::_is_edge_symbol(char c)
{
	return !(c == ' '	||
		c == ','		||
		c == '\n'		||
		c == '\r'		||
		c == '\t'		||
		c == ndash		||
		c == mdash		||
		c == quoteopen	||
		c == quoteclose);
}

bool mechan::DialogDownloader::_is_repeated_symbol(char c)
{
	return !(c == ' '	||
		c == '\t'		||
		c == '\n'		||
		c == '\r'		||
		c == '!'		||
		c == '?'		||
		c == '-'		||
		c == '('		||
		c == ')'		||
		c == ellipsis	||
		c == ndash		||
		c == mdash);
}

void mechan::DialogDownloader::_push_char(char c)
{
	if (_request_new_dialog)
	{
		_dialog.push_back(std::string());
		_dialog.push_back(std::string());
		_dialog.back().push_back(c);
		_request_new_dialog = false;
		_request_new_message = false;
	}
	else if (_request_new_message)
	{
		_dialog.push_back(std::string());
		_dialog.back().push_back(c);
		_request_new_dialog = false;
		_request_new_message = false;
	}
	else _dialog.back().push_back(c);
}

void mechan::DialogDownloader::_plain_text_wait_line(char c)
{
	switch (c)
	{
	case '<':
		_state = State::tag_wait_close;
		_request_new_message = true;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		break;
	}
	if (_plain_count++ == max_plain_text) _request_new_dialog = true;
}

void mechan::DialogDownloader::_plain_text_wait_dash(char c)
{
	switch (c)
	{
	case '<':
		_state = State::tag_wait_close;
		_request_new_message = true;
		break;
	
	case '\n':
	case '\r':
	case '\t':
	case ' ':
		break;

	case '-':
	case ndash:
	case mdash:
		_state = State::direct_speech_wait_dot;
		break;

	default:
		_state = State::plain_text_wait_line;
	}
	if (_plain_count++ == max_plain_text) _request_new_dialog = true;
}

void mechan::DialogDownloader::_direct_speech_wait_dot(char c)
{
	switch (c)
	{

	case '<':
		_state = State::tag_wait_close;
		_request_new_message = true;
		break;
	
	case '.':
	case ellipsis:
	case ',':
	case '!':
	case '?':
		_state = State::direct_speech_wait_dash;
		_push_char(c);
		break;

	case '\r':
	case '\n':
		_state = State::plain_text_wait_dash;
		_request_new_message = true;
		break;

	default:
		_push_char(c);
	}

	if (_direct_count++ == max_direct_speech)
	{
		if (!_dialog.empty()) _dialog.pop_back();
		_request_new_dialog = true;
		_state = State::corrupted_wait_line;
	}
}

void mechan::DialogDownloader::_direct_speech_wait_dash(char c)
{
	switch (c)
	{

	case '<':
		_state = State::tag_wait_close;
		_request_new_message = true;
		break;
	
	case '.':
	case ellipsis:
	case ',':
	case '!':
	case '?':
	case ' ':
	case '\t':
		_push_char(c);
		break;

	case ndash:
	case mdash:
		_state = State::author_speech_wait_dot;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_request_new_message = true;
		break;

	default:
		_state = State::direct_speech_wait_dot;
		_push_char(c);
	}
	if (_direct_count++ == max_direct_speech)
	{
		if (!_dialog.empty()) _dialog.pop_back();
		_request_new_dialog = true;
		_state = State::corrupted_wait_line;
	}
}

void mechan::DialogDownloader::_author_speech_wait_dot(char c)
{
	switch (c)
	{
	case '<':
		_state = State::tag_wait_close;
		_request_new_message = true;
		break;

	case '.':
	case 0x85:
	case ',':
	case '!':
	case '?':
		_state = State::author_speech_wait_dash;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_request_new_message = true;
		break;
	}
	if (_author_count++ == max_author_speech)
	{
		if (!_dialog.empty()) _dialog.pop_back();
		_request_new_dialog = true;
		_state = State::corrupted_wait_line;
	}
}

void mechan::DialogDownloader::_author_speech_wait_dash(char c)
{
	switch (c)
	{
	case '<':
		_state = State::tag_wait_close;
		_request_new_message = true;
		break;

	case '.':
	case ellipsis:
	case ',':
	case '!':
	case '?':
	case ' ':
	case '\t':
		break;

	case ndash:
	case mdash:
		_state = State::direct_speech_wait_dot;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_request_new_message = true;
		break;

	default:
		_state = State::author_speech_wait_dot;
	}
	if (_author_count++ == max_author_speech)
	{
		if (!_dialog.empty()) _dialog.pop_back();
		_request_new_dialog = true;
		_state = State::corrupted_wait_line;
	}
}

void mechan::DialogDownloader::_corrupred_wait_line(char c)
{
	switch (c)
	{
	case '<':
		_state = State::tag_wait_close;
		break;

	case '\n':
	case '\r':
		_state = State::plain_text_wait_dash;
		_plain_count = 0;
		_direct_count = 0;
		_author_count = 0;
		break;
	}
}

void mechan::DialogDownloader::_tad_wait_close(char c)
{
	if (c == '>')
	{
		_state = State::plain_text_wait_dash;
		_plain_count = 0;
		_direct_count = 0;
		_author_count = 0;
	}
}

void mechan::DialogDownloader::_parse()
{
	_state = State::plain_text_wait_line;
	_request_new_dialog = true;
	_request_new_message = true;
	_plain_count = 0;
	_direct_count = 0;
	_author_count = 0;

	char *text = _page_cp1251.data();
	while (*text != '\0')
	{
		char c;
		if (strcmp(text, "&quot;")	== 0) { c = '\"';  text += 6; }
		else if (*text == empty1 || *text == empty2 || *text == empty3) { c = ' '; text++; }
		else { c = *text; text++; }

		switch (_state)
		{
		case State::tag_wait_close: _tad_wait_close(c); break;
		case State::plain_text_wait_line: _plain_text_wait_line(c); break;
		case State::plain_text_wait_dash: _plain_text_wait_dash(c); break;
		case State::direct_speech_wait_dot: _direct_speech_wait_dot(c); break;
		case State::direct_speech_wait_dash: _direct_speech_wait_dash(c);  break;
		case State::author_speech_wait_dot: _author_speech_wait_dot(c); break;
		case State::author_speech_wait_dash: _author_speech_wait_dash(c); break;
		default: _corrupred_wait_line(c);
		}
	}
}

bool mechan::DialogDownloader::_download_page(const char *object)
{
	#ifdef _WIN32
		wchar_t object16[64];
		ir::Codec::recode<ir::Codec::UTF8, ir::Codec::UTF16>(object, ' ', object16);
		ir::HInternetResource get_request = WinHttpOpenRequest(_connection, L"GET", object16,
			NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
		if (get_request == NULL) return false;
		if (!WinHttpSendRequest(get_request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0)) return false;
		if (!WinHttpReceiveResponse(get_request, NULL)) return false;
		_page_utf8.resize(0);
		while (true)
		{
			DWORD size = 0;
			DWORD downloaded = 0;
			if (!WinHttpQueryDataAvailable(get_request, &size)) return false;
			if (size == 0) break;
			if (!_page_utf8.resize(_page_utf8.size() + size)) return false;
			if (!WinHttpReadData(get_request, &_page_utf8[_page_utf8.size() - size], size, &downloaded)) return false;
		}
	#else
		char command[128];
		if (_resource == Resource::ficbook_net) strcpy(command, "wget ficbook.net");
		else strcpy(command, "wget fanfics.me");
		strcat(command, object);
		strcat(command, " -O -");

		FILE *pipe = popen(command, "r");
		if (pipe == nullptr) return false;

		_page_utf8.resize(0);
		while (true)
		{
			if (!_page_utf8.reserve(_page_utf8.size() + 1024)) { pclose(pipe); return false; }
			size_t read = fread(_page_utf8.data() + _page_utf8.size(), 1, 1024, pipe);
			if (read == 0) break;
			_page_utf8.resize(_page_utf8.size() + read);
		}
		pclose(pipe);
	#endif

	if (_page_utf8.size() < 1024) return false;
	if (!_page_utf8.push_back('\0')) return false;
	if (_resource == Resource::ficbook_net)
	{
		if (strstr(_page_utf8.data(), "error404.svg") != nullptr) return false;
	}
	else
	{
		if (strstr(_page_utf8.data(), "HTTP ERROR 404") != nullptr) return false;
	}
	if (!_page_cp1251.resize(ir::Codec::recode<ir::Codec::UTF8, ir::Codec::CP1251>(_page_utf8.data(), ' ', nullptr) + 1)) return false;
	ir::Codec::recode<ir::Codec::UTF8, ir::Codec::CP1251>(_page_utf8.data(), ' ', _page_cp1251.data());
	if (!_page_cp1251.push_back('\0')) return false;
	return true;
}

bool mechan::DialogDownloader::_contains(const char *word, bool exact)
{
	size_t sublen = strlen(word);
	if (_page_cp1251.size() < sublen) return false;
	for (size_t i = 0; i < _page_cp1251.size() - sublen; i++)
	{
		bool match = true;
		for (size_t j = 0; j < sublen; j++)
		{
			if ((exact && _page_cp1251[i + j] != word[j])
			|| (!exact && mechan::lowercase(_page_cp1251[i + j]) != word[j])) { match = false; break; }
		}
		if (match)
		{
			return true;
		}
	}
	return false;
}

bool mechan::DialogDownloader::_filter_head()
{
	//if (_contains("\xCF\xEE\xE3\xF1\xE5\xE4\xED\xE5\xE2\xED\xEE\xF1\xF2\xFC"))	return false;	//Повседневность
	if (_resource == Resource::ficbook_net)
	{
		if (!_contains("\xCE\xF0\xE8\xE4\xE6\xE8\xED\xE0\xEB", true))					return false;	//Ориджинал
		if (_contains("\xD1\xF2\xE0\xF2\xFC\xE8", true))								return false;	//Статьи
		if (_contains("\xD4\xFD\xED\xF2\xE5\xE7\xE8", true))							return false;	//Фэнтези
		if (!_contains("\xC3\xE5\xF2", true) && !_contains("\xC4\xE6\xE5\xED", true))	return false;	//Гет, Джен
		if (_contains("\xD1\xF2\xE8\xF5\xE8", true))									return false;	//Стихи
		if (_contains("\xD1\xEB\xFD\xF8", true))										return false;	//Слэш
		if (_contains("\xD4\xE5\xEC\xF1\xEB\xFD\xF8", true))							return false;	//Фемслэш
		if (_contains("\xCE\xEC\xE5\xE3\xE0\xE2\xE5\xF0\xF1", true))					return false;	//Омегаверс
		if (_contains("PWP", true))		return false;
		if (_contains("NC-17", true))	return false;
		if (_contains("NC-21", true))	return false;
		if (_contains("18+", true))		return false;
	}
	else
	{
		if (!_contains("\xCE\xF0\xE8\xE4\xE6\xE8\xED\xE0\xEB</title>", true))			return false;	//Ориджинал</title>
		if (_contains("\xD1\xF2\xE0\xF2\xFC\xFF", true))								return false;	//Статья
		if (_contains("\xD4\xFD\xED\xF2\xE5\xE7\xE8", true))							return false;	//Фэнтези
		if (!_contains("(\xE3\xE5\xF2)", true)&&!_contains("(\xE4\xE6\xE5\xED)", true))	return false;	//(гет), (джен)
		if (_contains("\xCE\xEC\xE5\xE3\xE0\xE2\xE5\xF0\xF1", true))					return false;	//Омегаверс
		if (_contains("\xCD\xE5\xF6\xE5\xED\xE7\xF3\xF0\xED\xFB\xE5", true))			return false;	//Нецензурные
		if (_contains("NC-17", true))	return false;
	}
	return true;
}

bool mechan::DialogDownloader::_filter_body()
{
	/*
	char dash_space[3] = { mdash, ' ',  '\0' };
	size_t dash_space_count = 0;
	char *entry = _page_cp1251.data();
	while (true)
	{
		entry = strstr(entry, dash_space);
		if (entry == nullptr) break;
		dash_space_count++;
		entry++;
	}

	char minus_space[3] = { '-', ' ',  '\0' };
	size_t minus_space_count = 0;
	entry = _page_cp1251.data();
	while (true)
	{
		entry = strstr(entry, minus_space);
		if (entry == nullptr) break;
		minus_space_count++;
		entry++;
	}
	if (minus_space_count > dash_space_count) return false;
	*/

	if (_contains("\xE1\xEB\xFF"))			return false;	//blya (бля)
	if (_contains("\xF1\xF3\xEA\xE0"))		return false;	//suka (сука)
	if (_contains("\xEF\xE8\xE7\xE4"))		return false;	//pizd (пизд)
	if (_contains("\xF5\xF3\xE9"))			return false;	//hui (хуй)
	if (_contains("\xF5\xF3\xE5"))			return false;	//hue (хуе)
	if (_contains("\xE5\xE1\xE0\xED"))		return false;	//eban (ебан)
	if (_contains("\xE6\xEE\xEF"))			return false;	//zopa (жопа)
	if (_contains("\xF5\xE5\xF0"))			return false;	//her (хер)
	if (_contains("\xEC\xE8\xED\xE5\xF2"))	return false;	//minet (минет)
	return true;
}

mechan::DialogDownloader::DialogDownloader(Resource resource)
{
	_resource = resource;

	#ifdef _WIN32
		_session = WinHttpOpen(L"mechan_dialog_downloader",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (_session == NULL) return;
		const wchar_t *site = resource == Resource::ficbook_net ? L"ficbook.net" : L"fanfics.me";
		_connection = WinHttpConnect(_session, site, INTERNET_DEFAULT_HTTPS_PORT, 0);
		if (_connection == NULL) return;
	#endif

	_file = fopen("data\\dialog.txt", "ab");
}

void mechan::DialogDownloader::_write()
{
	//Cleaning edge and repeated symbols
	for (size_t i = 0; i < _dialog.size(); i++)
	{
		while (!_dialog[i].empty() && !_is_edge_symbol(_dialog[i].front())) _dialog[i].erase(_dialog[i].begin());
		size_t j = 0;
		while ((j + 1) < _dialog[i].size())
		{
			if (_dialog[i][j] == _dialog[i][j + 1] && !_is_repeated_symbol(_dialog[i][j])) _dialog[i].erase(_dialog[i].begin() + j + 1);
			else j++;
		}
		while (!_dialog[i].empty() && !_is_edge_symbol(_dialog[i].back())) _dialog[i].erase(_dialog[i].end() - 1);
	}

	//Cleaning single phrases
	for (size_t i = 0; i < _dialog.size(); i++)
	{
		if ((i == 0 || _dialog[i - 1] == "") &&
			(i == (_dialog.size() - 1) || _dialog[i + 1] == "") &&
			_dialog[i] != "") _dialog.erase(_dialog.begin() + i);
	}

	//Cleaning double spaces
	while (!_dialog.empty() && _dialog.front() == "") _dialog.erase(_dialog.begin());
	size_t j = 0;
	while ((j + 1) < _dialog.size())
	{
		if (_dialog[j] == "" && _dialog[j + 1] == "") _dialog.erase(_dialog.begin() + j + 1);
		else j++;
	}
	while (!_dialog.empty() && _dialog.back() == "") _dialog.erase(_dialog.end() - 1);

	//Writing to file
	printf("%u phrases written\n", (unsigned int)_dialog.size());
	for (size_t i = 0; i < _dialog.size(); i++)
	{
		fprintf(_file, "%s\n", _dialog[i].data());
	}
	if (_dialog.size() != 0) fprintf(_file, "\n");
	_dialog.resize(0);
}

void mechan::DialogDownloader::download()
{
	unsigned int fanfic_number = _resource == Resource::ficbook_net ? 0 : 65400;
	unsigned int fail_count = 0;

	while (fail_count < 3000)
	{
		printf("Processing fanfic #%u... ", ++fanfic_number);

		//Forming fic page with /
		char page_name[64];
		size_t page_name_len;
		if (_resource == Resource::ficbook_net) page_name_len = sprintf(page_name, "/readfic/%u/", fanfic_number);
		else page_name_len = sprintf(page_name, "/fic%u/", fanfic_number);

		//Downoading and filtering page
		if (!_download_page(page_name)) { printf("Failed\n"); fail_count++; continue; }
		else if (!_filter_head()) { printf("Rejected\n"); fail_count = 0; continue; }
		else { printf("Accepted\n"); fail_count = 0; }

		if (_resource == Resource::ficbook_net)
		{
			//Searching for references to chapters
			std::vector<unsigned int> chapter_numbers;
			char *entry = _page_cp1251.data();
			while (true)
			{
				entry = strstr(entry, page_name);
				if (entry == nullptr) break;
				unsigned int chapter_number = strtol(entry + page_name_len, &entry, 10);
				if (chapter_number != 0 && (chapter_numbers.empty() || chapter_numbers.back() != chapter_number))
					chapter_numbers.push_back(chapter_number);
				entry++;
			}

			//Processing
			if (chapter_numbers.empty())
			{
				if (_filter_body()) { printf("Chapter parsed\n"); _parse(); }
			}
			else for (unsigned int i = 0; i < chapter_numbers.size(); i++)
			{
				sprintf(page_name + page_name_len, "%u", chapter_numbers[i]);
				if (_download_page(page_name) && _filter_body()) { printf("Chapter parsed\n"); _parse(); }
			}
			_write();
		}
		else
		{
			for (unsigned int i = 0; true; i++)
			{
				sprintf(page_name, "read.php?id=%u&chapter=%u", fanfic_number, i);
				if (!_download_page(page_name)) break;
				if (_filter_body()) { printf("Chapter parsed\n"); _parse(); }
			}
			_write();
		}
	}
}

mechan::DialogDownloader::~DialogDownloader()
{
	#ifdef _WIN32
		if (_connection != NULL) WinHttpCloseHandle(_connection);
		if (_session != NULL) WinHttpCloseHandle(_session);
		if (_file != nullptr) fclose(_file);
	#endif
}

int _main()
{
	mechan::DialogDownloader downloader(mechan::DialogDownloader::Resource::fanfics_me);
	downloader.download();
	return 0;
}

int main()
{
	return _main();
}
