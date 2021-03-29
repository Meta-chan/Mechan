#define IR_INCLUDE 'i'
#include "../header/mechan_neuro.h"

int main()
{
	mechan::Dialog dialog;
	if (!dialog.ok()) return 1;
	mechan::Word word(&dialog);
	if (!word.ok()) return 1;
	mechan::Neuro neuro(&dialog, &word, true);
	if (!neuro.ok()) return 1;

	while (true)
	{
		neuro.train();
	}
}