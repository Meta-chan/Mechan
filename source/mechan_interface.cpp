#include "../header/mechan_interface.h"

mechan:Address::Address() :
	interface_id(Interface::ID::console),
	chat_id(0),
	user_id(0)
{
}

mechan::Address::Address(Interface::ID interface_id, unsigned long long int chat_id, unsigned long long int user_id)
	interface_id(interface_ide),
	chat_id(chat_id),
	user_id(user_id)
{
}