#ifndef _DEBUG

#define MECHAN_API_ID		1367564
#define MECHAN_API_HASH		"582ffce43327426097c4e71833e6e00f"
#define MECHAN_PHONE		"+491796171879"
#define MECHAN_FIRST_NAME	"Me-chan"
#define MECHAN_SECOND_NAME	""
#define MECHAN_PASSWORD		""

#include "../header/mechan_socket.h"
#include "../header/mechan_directory.h"
#include "../header/mechan_parse.h"

#include <td/telegram/Client.h>
#include <td/telegram/Log.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <time.h>
#include <stdio.h>
#include <limits>
#include <chrono>
#include <thread>
#define IR_IMPLEMENT
#include <ir_codec.h>

#define TDP(TDCLASSNAME) ::td::td_api::object_ptr<::td::td_api::TDCLASSNAME>
#define TDMOVE(TDOBJECT) ::td::td_api::move_object_as<decltype(TDOBJECT)::element_type>(TDOBJECT)
#define TDMOVEAS(TDCLASSNAME, TDOBJECT) ::td::td_api::move_object_as<::td::td_api::TDCLASSNAME>(TDOBJECT)
#define TDMAKE(TDCLASSNAME) ::td::td_api::make_object<td::td_api:: TDCLASSNAME>

namespace mechan
{
	class TelegramInterface
	{
	private:
		enum Status
		{
			pendling,
			authorized,
			failed
		};
		
		TDP(Object) _call(TDP(Function) function);
		TDP(Object) _process_autorization_state_wait_tdlib_parameters();
		TDP(Object) _process_autorization_state_wait_encryption_key();
		TDP(Object) _process_autorization_state_wait_phone_number();
		TDP(Object) _process_autorization_state_wait_code();
		TDP(Object) _process_autorization_state_wait_registration();
		TDP(Object) _process_autorization_state_wait_password();
		void _process_update_authorization_state(TDP(updateAuthorizationState) update_authorization_state);
		void _process_update_new_message_text(TDP(message) message);
		void _process_update_new_message(TDP(updateNewMessage) update_new_message);
		void _process_message(TDP(Object) object);
		bool _send(const std::string message, Client::Address address);
		void _receive();
		
		Status _status							= Status::pendling;
		std::int64_t _request_number			= 1;
		std::unique_ptr<::td::Client> _client	= nullptr;
		std::vector<::td::Client::Response> _responses;
		Client _mechan_client;
		Client::ReceiveResult _receive_result;

	public:		
		TelegramInterface();
		int loop();
		~TelegramInterface();
	};
}

TDP(Object) mechan::TelegramInterface::_call(TDP(Function) function)
{
	td::Client::Request request;
	if (_request_number < 0) _request_number = 1;
	request.id = _request_number++;
	request.function = TDMOVE(function);
	_client->send(std::move(request));
	
	while(true)
	{
		td::Client::Response response = _client->receive(1.0);
		if ((response.object == nullptr) || (response.id != 0 && response.id != request.id))
		{
			return nullptr;
		}
		else if (response.id == 0)
		{
			_responses.push_back(std::move(response));
		}
		else
		{
			return TDMOVE(response.object);
		}
	}
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_tdlib_parameters()
{
	printf("Processing authorizationStateWaitParameters\n");
	TDP(tdlibParameters) parameters = TDMAKE(tdlibParameters)();
	parameters->database_directory_ = MECHAN_DIR;
	parameters->use_message_database_ = true;
	parameters->use_secret_chats_ = false;
	parameters->api_id_ = MECHAN_API_ID;
	parameters->api_hash_ = MECHAN_API_HASH;
	parameters->system_language_code_ = "en";
	parameters->device_model_ = "Desktop";
	parameters->system_version_ = "Tiny Core Linux 11";
	parameters->application_version_ = "1.0";
	parameters->enable_storage_optimizer_ = true;
	return _call(TDMAKE(setTdlibParameters)(TDMOVE(parameters)));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_encryption_key()
{
	printf("Processing authorizationStateWaitEncryptionKey\n");
	return _call(TDMAKE(checkDatabaseEncryptionKey)(""));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_phone_number()
{
	printf("Processing authorizationStateWaitPhoneNumber\n");
	TDP(phoneNumberAuthenticationSettings) settings = TDMAKE(phoneNumberAuthenticationSettings)(true, false, false);
	return _call(TDMAKE(setAuthenticationPhoneNumber)(MECHAN_PHONE, TDMOVE(settings)));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_code()
{
	printf("Processing authorizationStateWaitCode\n");
	printf("Waiting confirmation code\n");
	char code[64];
	scanf("%64s", code);
	return _call(TDMAKE(checkAuthenticationCode)(code));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_registration()
{
	printf("Processing authorizationStateWaitRegistration\n");
	return _call(TDMAKE(registerUser)(MECHAN_FIRST_NAME, MECHAN_SECOND_NAME));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_password()
{
	printf("Processing authorizationStateWaitPassword\n");
	return _call(TDMAKE(checkAuthenticationPassword)(MECHAN_PASSWORD));
}

void mechan::TelegramInterface::_process_update_authorization_state(TDP(updateAuthorizationState) update_authorization_state)
{
	switch (update_authorization_state->authorization_state_->get_id())
	{			
	case td::td_api::authorizationStateWaitTdlibParameters::ID:
		_process_autorization_state_wait_tdlib_parameters();
		break;
		
	case td::td_api::authorizationStateWaitEncryptionKey::ID:
		_process_autorization_state_wait_encryption_key();
		break;
	
	case td::td_api::authorizationStateWaitPhoneNumber::ID:
		_process_autorization_state_wait_phone_number();
		break;

	case td::td_api::authorizationStateWaitCode::ID:
		_process_autorization_state_wait_code();
		break;
	
	case td::td_api::authorizationStateWaitRegistration::ID:
		_process_autorization_state_wait_registration();
		break;

	case td::td_api::authorizationStateWaitPassword::ID:
		_process_autorization_state_wait_password();
		break;

	case td::td_api::authorizationStateReady::ID:
		printf("Got authorization\n");
		_status = Status::authorized;
		break;
	}
}

void mechan::TelegramInterface::_process_update_new_message_text(TDP(message) message)
{
	//Setting user_id and quitting if sended by myself
	if (message->sender_->get_id() == td::td_api::messageSenderUser::ID)
	{
		TDP(messageSenderUser) message_sender_user = TDMOVEAS(messageSenderUser, message->sender_);
		TDP(Object) object = _call(TDMAKE(getUser)(message_sender_user->user_id_));
		TDP(user) user = TDMOVEAS(user, object);
		if (user->phone_number_ != "" && strstr(MECHAN_PHONE, user->phone_number_.data()) != nullptr) return;
		_receive_result.address.user_id = message_sender_user->user_id_;
	}
	else _receive_result.address.user_id = 0;
	
	//Setting other fields
	TDP(messageText) message_text = TDMOVEAS(messageText, message->content_);
	_receive_result.message = message_text->text_->text_;
	_receive_result.address.chat_id = message->chat_id_;
	_receive_result.ok = true;	
}

void mechan::TelegramInterface::_process_update_new_message(TDP(updateNewMessage) update_new_message)
{
	switch (update_new_message->message_->content_->get_id())
	{
	case td::td_api::messageText::ID:
		_process_update_new_message_text(TDMOVE(update_new_message->message_));
		break;
	}
}

void mechan::TelegramInterface::_process_message(TDP(Object) object)
{
	switch (object->get_id())
	{
	case td::td_api::updateAuthorizationState::ID:
		_process_update_authorization_state(TDMOVEAS(updateAuthorizationState, object));
		break;	
		
	case td::td_api::updateNewMessage::ID:
		_process_update_new_message(TDMOVEAS(updateNewMessage, object));
		break;
	}
}

mechan::TelegramInterface::TelegramInterface()
{
	if (!_mechan_client.ok()) return;

	td::Log::set_verbosity_level(1);
	_client = std::make_unique<::td::Client>();
	while (_status == Status::pendling)
	{
		td::Client::Response response;
		if (_responses.size() == 0)
		{
			printf("Waiting for response\n");
			response = _client->receive(std::numeric_limits<double>::infinity());
		}
		else
		{
			printf("Picking response from queue\n");
			response = std::move(_responses[0]);
			_responses.erase(_responses.begin());
		}
		if (response.object != nullptr) _process_message(TDMOVE(response.object));
	}
}

bool mechan::TelegramInterface::_send(const std::string message, Client::Address address)
{
	//Sending message
	TDP(sendMessage) sendmessage = TDMAKE(sendMessage)();
	sendmessage->chat_id_ = address.chat_id;
	sendmessage->reply_to_message_id_ = 0;
	sendmessage->options_ = TDMAKE(messageSendOptions)();
	sendmessage->options_->disable_notification_ = false;
	sendmessage->options_->from_background_ = false;
	TDP(formattedText) formattedtext = TDMAKE(formattedText)();
	formattedtext->text_ = message;
	sendmessage->input_message_content_ = TDMAKE(inputMessageText)(TDMOVE(formattedtext), false, false);
	TDP(Object) result = _call(TDMOVE(sendmessage));
	return result != nullptr;
}

void mechan::TelegramInterface::_receive()
{
	//Processing all incoming messages and looking on _receive_result
	_receive_result.ok = false;
	_receive_result.address.chat_id = 0;
	_receive_result.address.user_id = 0;
	_receive_result.message = "";

	while (true)
	{
		td::Client::Response response;
		if (_responses.empty())
		{
			response = _client->receive(std::numeric_limits<double>::infinity());
		}
		else
		{
			response = std::move(_responses[0]);
			_responses.erase(_responses.begin());
		}
		if (response.object != nullptr) _process_message(TDMOVE(response.object));
		if (_receive_result.ok) break;
	}
}

int mechan::TelegramInterface::loop()
{
	if (!_mechan_client.ok() || _status != Status::authorized) return 1;
	while (true)
	{
		//Receiving message
		_receive();
		if (!_receive_result.ok || _receive_result.message.empty()) continue;		
		printf("Got message\n");
		std::string question;
		question.resize(ir::Codec::recode<ir::Codec::UTF8, ir::Codec::CP1251>(_receive_result.message.data(), ' ', nullptr));
		ir::Codec::recode<ir::Codec::UTF8, ir::Codec::CP1251>(_receive_result.message.data(), ' ', &question[0]);
			
		//Sending to Mechan
		_mechan_client.send(question, _receive_result.address);
		
		//Receiving from Mechan
		while (true)
		{
			_receive_result = _mechan_client.receive();
			if (_receive_result.ok) break;
			else std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::string answer8;
		answer8.resize(ir::Codec::recode<ir::Codec::CP1251, ir::Codec::UTF8>(_receive_result.message.data(), ' ', nullptr));
		ir::Codec::recode<ir::Codec::CP1251, ir::Codec::UTF8>(_receive_result.message.data(), ' ', &answer8[0]);
		
		//Sending message
		_send(answer8, _receive_result.address);
		printf("Sent result\n");
		
		//Post-processing commands
		if (_receive_result.message.front() == '!' || _receive_result.message.front() == '?')
		{
			std::vector<std::string> parsed;
			parse_space(_receive_result.message, &parsed);
			// !shutdown
			if (result.message.front() == '!'
				&& parsed.size() == 1
				&& parsed[0] == "shutdown")
			{
				_server.send("!", result.address);
				return 0;
			}
		}
	}
}

mechan::TelegramInterface::~TelegramInterface()
{
}

int _main()
{
	mechan::TelegramInterface telegram;
	return telegram.loop();
}

int main()
{
	return _main();
}

#endif
