#define MECHAN_API_ID		1367564
#define MECHAN_API_HASH		"582ffce43327426097c4e71833e6e00f"
#define MECHAN_PHONE		"+491796171879"
#define MECHAN_FIRST_NAME	"Me-chan"
#define MECHAN_SECOND_NAME	""
#define MECHAN_PASSWORD		""

#include "../header/mechan_common.h"
#include "../header/mechan_telegram_interface.h"
#include "../header/mechan_console_interface.h"
#include "../header/mechan_log_interface.h"

#include <td/telegram/Client.h>
#include <td/telegram/Log.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <time.h>

#define TDP(TDCLASSNAME) ::td::td_api::object_ptr<::td::td_api::TDCLASSNAME>
#define TDMOVE(TDOBJECT) ::td::td_api::move_object_as<decltype(TDOBJECT)::element_type>(TDOBJECT)
#define TDMOVEAS(TDCLASSNAME, TDOBJECT) ::td::td_api::move_object_as<::td::td_api::TDCLASSNAME>(TDOBJECT)
#define TDMAKE(TDCLASSNAME) ::td::td_api::make_object<td::td_api:: TDCLASSNAME>

namespace mechan
{
	class TelegramInterface : public Interface
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
	
		Status _status							= Status::pendling;
		bool _autorization_failed				= false;
		std::int64_t _request_number			= 1;
		std::unique_ptr<::td::Client> _client	= nullptr;
		std::vector<::td::Client::Response> _responses;
		ReadResult _read_result;

	public:		
		TelegramInterface();
		bool ok() const noexcept;
		Interface::ID id() const noexcept;
		bool write(const std::string message, Address address);
		ReadResult read(unsigned int timeout);
		~TelegramInterface();
	};
}

mechan::Interface *mechan::new_telegram_interface()
{
	return new TelegramInterface;
}

mechan::Interface *mechan::telegram_interface;

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
	log_interface->write("Processing authorizationStateWaitParameters\n");
	TDP(tdlibParameters) parameters = TDMAKE(tdlibParameters)();
	parameters->database_directory_ = MECHAN_DIR;
	parameters->use_message_database_ = true;
	parameters->use_secret_chats_ = false;
	parameters->api_id_ = MECHAN_API_ID;
	parameters->api_hash_ = MECHAN_API_HASH;
	parameters->system_language_code_ = "en";
	parameters->device_model_ = "Desktop";
	parameters->system_version_ = "Windows 8";
	parameters->application_version_ = "1.0";
	parameters->enable_storage_optimizer_ = true;
	return _call(TDMAKE(setTdlibParameters)(TDMOVE(parameters)));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_encryption_key()
{
	log_interface->write("Processing authorizationStateWaitEncryptionKey\n");
	return _call(TDMAKE(checkDatabaseEncryptionKey)(""));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_phone_number()
{
	log_interface->write("Processing authorizationStateWaitPhoneNumber\n");
	TDP(phoneNumberAuthenticationSettings) settings = TDMAKE(phoneNumberAuthenticationSettings)(true, false, false);
	return _call(TDMAKE(setAuthenticationPhoneNumber)(MECHAN_PHONE, TDMOVE(settings)));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_code()
{
	log_interface->write("Processing authorizationStateWaitCode\n");
	console_interface->write("Waiting confirmation code\n");
	ReadResult result = console_interface->read();
	return _call(TDMAKE(checkAuthenticationCode)(result.message));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_registration()
{
	log_interface->write("Processing authorizationStateWaitRegistration\n");
	return _call(TDMAKE(registerUser)(MECHAN_FIRST_NAME, MECHAN_SECOND_NAME));
}

TDP(Object) mechan::TelegramInterface::_process_autorization_state_wait_password()
{
	log_interface->write("Processing authorizationStateWaitPassword\n");
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
		log_interface->write("Got authorization\n");
		console_interface->write("Got authorization\n");
		_status = Status::authorized;
		break;
	}
}

void mechan::TelegramInterface::_process_update_new_message_text(TDP(message) message)
{
	TDP(messageText) messagetext = TDMOVEAS(messageText, message->content_);
	_read_result.ok = true;
	_read_result.message = messagetext->text_->text_;
	_read_result.address.interface_id = Interface::ID::telegram;
	_read_result.address.chat_id = message->chat_id_;
	if (message->sender_->get_id() == td::td_api::messageSenderUser::ID)
	{
		TDP(messageSenderUser) message_sender_user = TDMOVEAS(messageSenderUser, message->sender_);
		_read_result.address.user_id = message_sender_user->user_id_;
	}
	else _read_result.address.user_id = 0;
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
	while (_status == Status::pendling)
	{
		td::Client::Response response;
		if (_responses.empty())
		{
			response = _client->receive(INFINITY);
		}
		else
		{
			response = std::move(_responses[0]);
			_responses.erase(_responses.begin());
		}
		if (response.object != nullptr) _process_message(TDMOVE(response.object));
	}
}

bool mechan::TelegramInterface::ok() const noexcept
{
	return _status == Status::authorized;
}

mechan::Interface::ID mechan::TelegramInterface::id() const noexcept
{
	return Interface::ID::telegram;
}

bool mechan::TelegramInterface::write(const std::string message, Address address)
{
	//Sending message
	TDP(sendMessage) sendmessage = TDMAKE(sendMessage)();
	sendmessage->chat_id_ = 0;
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

mechan::Interface::ReadResult mechan::TelegramInterface::read(unsigned int timeout)
{
	_read_result.ok = false;
	clock_t c = clock();
	while (true)
	{
		td::Client::Response response;
		if (_responses.empty())
		{
			response = _client->receive(0.001 * timeout);
		}
		else
		{
			response = std::move(_responses[0]);
			_responses.erase(_responses.begin());
		}
		if (response.object != nullptr) _process_message(TDMOVE(response.object));
		if (_read_result.ok || ((unsigned int)(clock() - c) > timeout)) return _read_result;
	}
}