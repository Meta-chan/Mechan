#define IR_INCLUDE 'i'
#include "../header/mechan_socket.h"
#include <ir/encoding.h>
#include <stdio.h>
#include <iostream>

int main()
{
	mechan::TCPClient client;
	mechan::TCPAddress address;

	if (!client.ok()) return 1;
	while (true)
	{
		//Reading question
		std::string question866;
		std::getline(std::cin, question866);
		std::string question;
		question.resize(question866.size());
		ir::Encoding::recode<ir::Encoding::CP866, ir::Encoding::CP1251>(&question[0], question866.data(), "");

		//Sending question
		client.send(address, question);
		
		//Receiving answer
		std::string answer;
		if (client.receive(&address, &answer))
		{
			//Printing answer
			std::string answer866;
			answer866.resize(answer.size());
			ir::Encoding::recode<ir::Encoding::CP1251, ir::Encoding::CP866>(&answer866[0], answer.c_str(), "");
			std::cout << answer866 << std::endl;
		}		
	}
}