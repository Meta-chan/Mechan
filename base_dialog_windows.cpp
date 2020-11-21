#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <winhttp.h>
#include <stdlib.h>
#pragma comment(lib, "winhttp.lib")

#define IR_IMPLEMENT
#include <ir_resource/ir_hinternet_resource.h>
#include <ir_errorcode.h>
#include <ir_database/ir_n2st_database.h>
#include <ir_container/ir_quiet_vector.h>
#include <ir_utf.h>

#include "header/dialog_extracter.h"

ir::QuietVector<char> fic, part, utf8;

HINTERNET hSession = NULL;
HINTERNET hConnect = NULL;
ir::N2STDatabase *base = nullptr;
DialogExtracter extracter;

bool getdata(HINTERNET hConnect, const wchar_t *object, ir::QuietVector<char> *data)
{
	ir::HInternetResource hGetRequest = WinHttpOpenRequest(hConnect, L"GET", object,
	NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,	WINHTTP_FLAG_SECURE);
	if (hGetRequest == NULL) return false;
	
	if (!WinHttpSendRequest(hGetRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0)) return false;

	if (!WinHttpReceiveResponse(hGetRequest, NULL)) return false;

	utf8.resize(0);
	while(true)
	{
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		if (!WinHttpQueryDataAvailable(hGetRequest, &dwSize)) return false;
		if (dwSize == 0) break;
		if (!utf8.resize(utf8.size() + dwSize)) return false;
		if (!WinHttpReadData(hGetRequest, &utf8[utf8.size() - dwSize], dwSize, &dwDownloaded)) return false;
	};

	if (!utf8.push_back('\0')) return false;

	unsigned int len = utf_recode(&utf_utf8, utf8.data(), ' ', &utf_1251, nullptr);
	if (!data->reserve(len + 1)) return false;
	utf_recode(&utf_utf8, utf8.data(), ' ', &utf_1251, data->data());
	return true;
};

bool init()
{
	utf_init();
	utf_utf8.init();
	utf_1251.init();

	ir::ec code;
	base = new ir::N2STDatabase(L"C:\\Users\\Dell E7240\\Desktop\\Dialog.ind", ir::Database::create_new_always, &code);
	if (code != ir::ec::ec_ok) return false;

	hSession = WinHttpOpen(L"Mechan_dialog_base",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession == NULL) return false;

	hConnect = WinHttpConnect(hSession, L"ficbook.net", INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (hConnect == NULL) return false;

	return true;
};

bool strstr_lower(const char *s, const char *sub)
{
	while (*s != '\0')
	{
		unsigned int i = 0;
		while(true)
		{
			if (sub[i] == '\0') return true;
			
			unsigned char c = s[i];
			if (c >= 0xC0 && c <= 0xDF) c = c + 0x20;
			if (c == 0xA8 || c == 0xB8) c = 0xE5;
			if (c == sub[i]) i++;
			else break;
		}
		s++;
	}
	return false;
};

bool swarefilter(const char *text)
{	

	if (strstr_lower(text, "бля")) return false;
	if (strstr_lower(text, "сука")) return false;
	if (strstr_lower(text, "пизд")) return false;
	if (strstr_lower(text, "хуй")) return false;
	if (strstr_lower(text, "хуе")) return false;
	if (strstr_lower(text, "ебан")) return false;
	if (strstr_lower(text, "жоп")) return false;
	if (strstr_lower(text, "хер")) return false;
	if (strstr_lower(text, "вагин")) return false;
	if (strstr_lower(text, "сперм")) return false;
	if (strstr_lower(text, "трах")) return false;
	if (strstr_lower(text, "минет")) return false;

	return true;
};

bool _main()
{
	char pagename[64] = { 0 };
	wchar_t pagenamew[64] = { 0 };
	unsigned int ficcount = 0;
	unsigned int failcount = 0;

	strcpy_s(pagename, "/readfic/");
	wcscpy_s(pagenamew, L"/readfic/");
	const unsigned int readficlen = strlen("/readfic/");
	while (failcount < 1000)
	{
		ficcount++;
		printf("Processing fanfic #%i... ", ficcount);
		if (GetKeyState(' ')) return true;

		//forming fic page with /
		_itoa(ficcount, pagename + readficlen, 10);
		const unsigned int numberlen = strlen(pagename + readficlen);
		pagename[readficlen + numberlen] = '/';
		pagename[readficlen + numberlen + 1] = '\0';

		//forming fic page without /
		_itow(ficcount, pagenamew + readficlen, 10);

		//loading page
		if (!getdata(hConnect, pagenamew, &fic)) { failcount++; continue; };
		failcount = 0;

		//skipping poems and slash
		if (strstr(fic.data(), "Стихи") != nullptr) { printf("rejected, poem\n"); continue; };
		if (strstr(fic.data(), "18+") != nullptr) { printf("rejected, 18+\n"); continue; };
		if (strstr(fic.data(), "Гет") == nullptr &&
			strstr(fic.data(), "Джен") == nullptr) { printf("rejected, slash/femslash\n"); continue; };
		
		//searching for references to parts
		bool manyparts = false;
		char *entry = fic.data();
		while(entry != nullptr)
		{
			entry = strstr(entry, pagename);
			if (entry != nullptr)
			{
				unsigned int partnumber = strtol(entry + readficlen + numberlen + 1, &entry, 10);
				if (partnumber != 0)
				{
					//forming part page name
					pagenamew[readficlen + numberlen] = '/';
					_itow(partnumber, pagenamew + readficlen + numberlen + 1, 10);
					if (getdata(hConnect, pagenamew, &part))
					{
						if (swarefilter(part.data()))
						{
							printf("accepted\n");
							extracter.parse(part.data(), base);
						}
						else printf("rejected, sware\n");
					}
					manyparts = true;
				}
			}
		}

		if (!manyparts)
		{
			if (swarefilter(fic.data()))
			{
				printf("accepted\n");
				extracter.parse(fic.data(), base);
			}
			else printf("rejected, sware\n");
		}
	}

	return true;
};

void finalize()
{
	if (base != nullptr) delete base;
	if (hConnect != NULL) WinHttpCloseHandle(hConnect);
	if (hSession != NULL) WinHttpCloseHandle(hSession);
};

int main()
{	
	if(init())_main();
	finalize();
};