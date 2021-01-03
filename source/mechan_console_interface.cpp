#include "../header/mechan_socket.h"
#define IR_IMPLEMENT
#include <ir_codec.h>
#include <stdio.h>
#include <iostream>

int main()
{
	mechan::Client client;
	if (!client.ok()) return 1;
	while (true)
	{
		std::string question866;
		std::getline(std::cin, question866);
		std::string question;
		question.resize(question866.size());
		ir::Codec::recode<ir::Codec::CP866, ir::Codec::CP1251>(question866.data(), ' ', &question[0]);
		client.send(question, mechan::Client::Address());
		mechan::Client::ReceiveResult result;
		while (true)
		{
			result = client.receive();
			if (result.ok) break;
		}
		std::string result866;
		result866.resize(result.message.size());
		ir::Codec::recode<ir::Codec::CP1251, ir::Codec::CP866>(result.message.data(), ' ', &result866[0]);
		std::cout << result866 << std::endl;
	}
}