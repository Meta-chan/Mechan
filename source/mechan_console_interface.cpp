#include "../header/mechan_socket.h"
#include <stdio.h>
#include <iostream>

int main()
{
	mechan::Client client;
	if (!client.ok()) return 1;

	std::string question;
	while (true)
	{
		std::getline(std::cin, question);
		client.send(question, mechan::Client::Address());
		mechan::Client::ReceiveResult result;
		while (true)
		{
			result = client.receive();
			if (result.ok) break;
		}
		std::cout << result.message << std::endl;
	}
}