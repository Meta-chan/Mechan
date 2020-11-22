#define IR_IMPLEMENT
#include <ir_utf.h>
#include <ir_database/ir_s2st_database.h>
#include <ir_resource/ir_file_resource.h>
#include <ir_resource/ir_memresource.h>

bool readutf8(FILE *file, char *buffer)
{
	while (fread(&buffer[0], 1, 1, file) != 0 && (buffer[0] == ' ' || buffer[0] == '\n')) {}

	unsigned int i = 1;
	while (true)
	{
		if (fread(&buffer[i], 1, 1, file) == 0 || buffer[i] == ' ' || buffer[i] == '\n')
		{
			buffer[i] = '\0';
			return true;
		}
		i++;
	}
}

bool read1251(FILE *file, char *buffer)
{
	char utf8buffer[128];
	if (!readutf8(file, utf8buffer)) return false;
	utf_recode(&utf_utf8, utf8buffer, ' ', &utf_1251, buffer);
	return (strstr(buffer, " ") == nullptr);
}

bool parse()
{
	utf_init();
	utf_utf8.init();
	utf_1251.init();

	ir::FileResource file = _wfsopen(L"C:\\Project\\_source\\Mechan\\Wikivectors.bin", L"rb", _SH_DENYNO);
	if (file.it == nullptr) return 0;

	ir::ec code;
	ir::S2STDatabase base(L"C:\\Project\\mechan\\database\\wikivectors.idt", ir::Database::create_new_always, &code);

	char buffer[128];
	if (!read1251(file.it, buffer)) return false;
	int nwords = strtol(buffer, nullptr, 10);
	if (nwords <= 0) return false;
	
	if (!read1251(file.it, buffer)) return false;
	int dim = strtol(buffer, nullptr, 10);
	if (dim <= 0) return false;

	ir::MemResource<float> vector = (float*)malloc(sizeof(float) * dim);
	if (vector.it == nullptr) return false;

	for (int i = 0; i < nwords; i++)
	{
		bool success = read1251(file.it, buffer);
		if (fread(vector.it, sizeof(float), dim, file.it) < dim) return false;
		if (success)
		{
			ir::ConstBlock key(strlen(buffer), buffer);
			ir::ConstBlock data(sizeof(float) * dim, vector.it);
			if (base.insert(key, data) != ir::ec::ec_ok) return false;
		}
	}
	return true;
};

int main()
{
	printf(parse() ? "Success" : "Fail");
	getchar();
	return 0;
};