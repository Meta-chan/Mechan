#define IR_IMPLEMENT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header/dialog_extracter.h"
#include <ir_resource/ir_resource.h>
#include <ir_database/ir_n2st_database.h>
#include <ir_utf.h>

struct CharBlock
{
	char *data		= nullptr;
	unsigned int reserved	= 0;
} fic, part, utf8;

ir::N2STDatabase *base = nullptr;
DialogExtracter extracter;

bool getdata(const char *object, CharBlock *data)
{
	char command[128];
	strcpy(command, "wget ficbook.net");
	strcat(command, object);
	strcat(command, " -O -");
	
	FILE *pipe = popen(command, "r");
	if (pipe == nullptr) return false;
	
	unsigned int used = 0;
	while (true)
	{
		if (!reserve((void**)&utf8.data, &utf8.reserved, used + 1024))
		{
			fclose(pipe);
			return false;
		}
		unsigned int read = fread(utf8.data + used, 1, 1024, pipe);
		if (read == 0) break;
		used += read;
	}
	pclose(pipe);

	if (!reserve((void**)&utf8.data, &utf8.reserved, used + 1)) return false;
	utf8.data[used] = '\0';

	unsigned int len = utf_recode(&utf_utf8, utf8.data, ' ', &utf_1251, nullptr);
	if (!reserve((void**)&data->data, &data->reserved, len + 1)) return false;
	utf_recode(&utf_utf8, utf8.data, ' ', &utf_1251, data->data);
	
	return true;
};

bool init()
{
	utf_init();
	utf_utf8.init();
	utf_1251.init();

	ir::ec code;
	base = new ir::N2STDatabase("/home/project/mechan_dialog_base/dialog.idb", ir::N2STDatabase::access_new, ir::N2STDatabase::rammode_no, &code);
	if (code != ir::ec::ec_ok) return false;

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

bool strstr_exact(const char *s, const char *sub)
{
	unsigned int sublen = strlen(sub);
	while (*s != '\0')
	{
		if (memcmp(s, sub, sublen) == 0) return true;
		else s++;
	}
	return false;
};

bool swarefilter(const char *text)
{	
	unsigned char swares[][8] = {
		{0xE1, 0xEB, 0xFF, 0},
		{0xF1, 0xF3, 0xEA, 0xE0, 0},
		{0xEF, 0xE8, 0xE7, 0xE4, 0},
		{0xF5, 0xF3, 0xE9, 0},
		{0xF5, 0xF3, 0xE5, 0},
		{0xE5, 0xE1, 0xE0, 0xED, 0},
		{0xE6, 0xEE, 0xEF, 0},
		{0xF5, 0xE5, 0xF0, 0},
		{0xE3, 0xE0, 0xE4, 0xE8, 0xED, 0},
		{0xF1, 0xEF, 0xE6, 0xF0, 0xEC, 0},
		{0xF2, 0xF0, 0xE0, 0xF5, 0},
		{0xEC, 0xE8, 0xED, 0xE5, 0xF2, 0}};

	for (unsigned int i = 0; i < 12; i++)
	{
		if (strstr_lower(text, (char*)swares[i])) return false;
	}

	return true;
};

bool _main()
{
	char pagename[64];
	strcpy(pagename, "/readfic/");
	unsigned int readficlen = strlen("/readfic/");
	unsigned int ficcount = 0;
	unsigned int failcount = 0;

	while (failcount < 1000)
	{
		ficcount++;	
		printf("Processing fanfic #%i... ", ficcount);
		
		if (ficcount % 1000 == 0)
		{
			FILE *stopfile = fopen("stop", "r");
			if (stopfile != nullptr)
			{
				fclose(stopfile);
				return true;
			}
		}

		//forming fic page with /
		sprintf(pagename + readficlen, "%u", ficcount);
		unsigned int ficcountlen = strlen(pagename + readficlen);
		pagename[readficlen + ficcountlen] = '/';
		pagename[readficlen + ficcountlen + 1] = '\0';

		//loading page
		if (!getdata(pagename, &fic)) { failcount++; continue; };
		failcount = 0;

		//skipping poems and slash
		const unsigned char janstr[] = {0xC4, 0xE6, 0xE5, 0xED, 0};
	      	const unsigned char poemstr[] = {0xD1, 0xF2, 0xE8, 0xF5, 0xE8, 0};
		const unsigned char getstr[] = {0xC3, 0xE5, 0xF2, 0};

		if (strstr_exact(fic.data, (char*)poemstr)) { printf("rejected, poem\n"); continue; };
		if (strstr_exact(fic.data, "18+")) { printf("rejected, 18+\n"); continue; };
		if (!strstr_exact(fic.data, (char*)getstr) &&
			!strstr_exact(fic.data, (char*)janstr)) { printf("rejected, slash/femslash\n"); continue; };
		
		//searching for references to parts
		bool manyparts = false;
		char *entry = fic.data;
		while(entry != nullptr)
		{
			entry = strstr(entry, pagename);
			if (entry != nullptr)
			{
				unsigned int partnumber = strtol(entry + readficlen + ficcountlen + 1, &entry, 10);
				if (partnumber != 0)
				{
					//adding part
					sprintf(pagename + readficlen + ficcountlen + 1, "%u", partnumber);
					if (getdata(pagename, &part))
					{
						if (swarefilter(part.data))
						{
							printf("accepted\n");
							extracter.parse(part.data, base);
						}
						else printf("rejected, sware\n");
					}
					manyparts = true;
				}
			}
		}

		if (!manyparts)
		{
			if (swarefilter(fic.data))
			{
				printf("accepted\n");
				extracter.parse(fic.data, base);
			}
			else printf("rejected, sware\n");
		}
	}

	return true;
};

void finalize()
{
	if (base != nullptr) delete base;
	if (utf8.data != nullptr) free(utf8.data);
	if (fic.data != nullptr) free(fic.data);
	if (part.data != nullptr) free(part.data);
};

int main()
{	
	if(init())_main();
	finalize();
};