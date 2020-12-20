#include "../header/mechan_socket.h"
#define IR_IMPLEMENT
#include <ir_utf.h>
#include <stdio.h>
#include <iostream>

int main()
{
	mechan::Client client;
	if (!client.ok()) return 1;
	ir_utf_init();
	ir_utf_866.init();
	ir_utf_1251.init();
	while (true)
	{
		std::string question866;
		std::getline(std::cin, question866);
		std::string question;
		question.resize(question866.size());
		ir_utf_recode(&ir_utf_866, question866.data(), ' ', &ir_utf_1251, &question[0]);
		client.send(question, mechan::Client::Address());
		mechan::Client::ReceiveResult result;
		while (true)
		{
			result = client.receive();
			if (result.ok) break;
		}
		std::string result866;
		result866.resize(result.message.size());
		ir_utf_recode(&ir_utf_1251, result.message.data(), ' ', &ir_utf_866, &result866[0]);
		std::cout << result866 << std::endl;
	}
}