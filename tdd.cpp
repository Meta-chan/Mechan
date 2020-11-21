//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <td/telegram/Client.h>
#include <td/telegram/Log.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>

#define TDP(TDCLASSNAME) ::td::td_api::object_ptr<::td::td_api::TDCLASSNAME>
#define TDMOVE(TDOBJECT) ::td::td_api::move_object_as<decltype(TDOBJECT)::element_type>(TDOBJECT)
#define TDMOVEAS(TDCLASSNAME, TDOBJECT) ::td::td_api::move_object_as<::td::td_api::TDCLASSNAME>(TDOBJECT)
#define TDMAKE(TDCLASSNAME) td::td_api::make_object<td::td_api:: TDCLASSNAME>

#define MECHAN_PIPE_DIR		"/tmp/mechan"
#define MECHAN_DIR			"/home/project/mechan/mechantdd/tdlib"
#define MECHAN_API_ID		1367564
#define MECHAN_API_HASH		"582ffce43327426097c4e71833e6e00f"
#define MECHAN_PHONE		"+491796171879"
#define MECHAN_FIRST_NAME	"Me-chan"
#define MECHAN_SECOND_NAME	""
#define MECHAN_PASSWORD		""
#define MECHAN_TIMEOUT		1.0f

class Mechan
{
private:
	bool _authorized					= false;
	std::int64_t _requestNumber				= 0;
	std::unique_ptr<::td::Client> _client			= nullptr;
	std::vector<::td::Client::Response> _messageQueue;
	int _log						= -1;
	int _readpipe						= -1;
	
	TDP(Object) _call(TDP(Function) function);
	std::string _getChatName(std::int64_t chatid);
	std::string _getUserName(std::int32_t userid);
	std::int64_t _dreadnumber(int filedes, char delim);

	TDP(Object) _processASWTdlibParameters();
	TDP(Object) _processASWEncryptionKey();
	TDP(Object) _processASWPhoneNumber();
	TDP(Object) _processASWCode();
	TDP(Object) _processASWRegistration();
	TDP(Object) _processASWPassword();
	void _processUpdateAuthorizationState(TDP(updateAuthorizationState) updateauthorizationstate);
	void _processUpdateNewMessageText(TDP(message) message);
	void _processUpdateNewMessage(TDP(updateNewMessage) updatenewmessage);
	void _processMessage(TDP(Object) object);
	unsigned int _processMessages();

	void _processPipeMessageQ();
	void _processPipeMessageT();
	void _processPipeMessage();
	unsigned int _processPipeMessages();

public:
	
	Mechan();
	void loop();
	~Mechan();
};

Mechan::Mechan()
{
	td::Log::set_verbosity_level(1);
	_client = std::make_unique<::td::Client>();
	
	//Creating folder (nowhere to print)
	mkdir(MECHAN_PIPE_DIR, 0755);
		
	//Creating log
	_log = open(MECHAN_PIPE_DIR "/log", O_WRONLY | O_CREAT, 0644);
	if (_log < 0) return;
	dprintf(_log, MECHAN_PIPE_DIR "/log created\n");
	
	//Creating read pipe
	mkfifo(MECHAN_PIPE_DIR "/response", 0644);
	dprintf(_log, MECHAN_PIPE_DIR "/response created created\n");
	_readpipe = open(MECHAN_PIPE_DIR "/response", O_RDONLY | O_NONBLOCK);
	if (_readpipe < 0) return;
	dprintf(_log, MECHAN_PIPE_DIR "/response opened\n");
	
	//Creating write pipe
	mkfifo(MECHAN_PIPE_DIR "/message", 0644);
	dprintf(_log, MECHAN_PIPE_DIR "/message created\n");
	
	//Authorizing
	while(!_authorized)
	{
		if (_processMessages() == 0)
		{
			dprintf(_log, "Not got authorization...");
			break;
		}
	}
};

TDP(Object) Mechan::_call(TDP(Function) function)
{
	td::Client::Request request;
	if (_requestNumber == 0) _requestNumber = 1;
	request.id = _requestNumber++;
	request.function = TDMOVE(function);
	_client->send(std::move(request));
	
	while(true)
	{
		td::Client::Response response = _client->receive(MECHAN_TIMEOUT);
		if ((response.object == nullptr) || ((response.id != request.id) && (response.id != 0))) //Not got or got wrong respone
		{
			return nullptr;
		}
		else if (response.id == 0)
		{
			_messageQueue.push_back(std::move(response));
		}
		else
		{
			return TDMOVE(response.object);
		}
	}
};

std::string Mechan::_getChatName(std::int64_t chatid)
{
	std::string chatname;
	if (chatid != 0)
	{
		TDP(chat) chat = TDMOVEAS(chat, _call(TDMAKE(getChat)(chatid))); //needs to be cached
		if (chat != nullptr) chatname = chat->title_;
	}
	else chatname = "unknown";
	return chatname;
};

std::string Mechan::_getUserName(std::int32_t userid)
{
	std::string username;
	if (userid != 0)
	{
		TDP(user) user = TDMOVEAS(user, _call(TDMAKE(getUser)(userid))); //needs to be cached
		if (user != nullptr) username = user->first_name_ + " " + user->last_name_;
	}
	else username = "unknown";
	return username;
};

std::int64_t Mechan::_dreadnumber(int fd, char delim)
{
	std::int64_t result = 0;
	while(true)
	{
		char c;
		int r = read(fd, &c, 1);
		if (r > 0 && c >= '0' && c <= '9') result = 10 * result + (c - '0');
		else if (c == delim) return result;
		else return 0;
	}
};

TDP(Object) Mechan::_processASWTdlibParameters()
{
	dprintf(_log, "Processing authorizationStateWaitParameters\n");
	TDP(tdlibParameters) parameters = TDMAKE(tdlibParameters)();
	parameters->database_directory_ = MECHAN_DIR;
	parameters->use_message_database_ = true;
	parameters->use_secret_chats_ = false;
	parameters->api_id_ = MECHAN_API_ID;
	parameters->api_hash_ = MECHAN_API_HASH;
	parameters->system_language_code_ = "en";
	parameters->device_model_ = "Desktop";
	parameters->system_version_ = "Tiny Core Linux";
	parameters->application_version_ = "1.0";
	parameters->enable_storage_optimizer_ = true;
	return _call(TDMAKE(setTdlibParameters)(TDMOVE(parameters)));
};

TDP(Object) Mechan::_processASWEncryptionKey()
{
	dprintf(_log, "Processing authorizationStateWaitEncryptionKey\n");
	return _call(TDMAKE(checkDatabaseEncryptionKey)(""));
};

TDP(Object) Mechan::_processASWPhoneNumber()
{
	dprintf(_log, "Processing authorizationStateWaitPhoneNumber\n");
	TDP(phoneNumberAuthenticationSettings) settings = TDMAKE(phoneNumberAuthenticationSettings)(true, false, false);
	return _call(TDMAKE(setAuthenticationPhoneNumber)(MECHAN_PHONE, TDMOVE(settings)));
};

TDP(Object) Mechan::_processASWCode()
{
	dprintf(_log, "Processing authorizationStateWaitCode\n");
	dprintf(_log, "Waiting the code to be written into " MECHAN_PIPE_DIR "/code\n");
	if (mkfifo(MECHAN_PIPE_DIR "/code", 0777) < 0) return nullptr;
	int codefile = open(MECHAN_PIPE_DIR "/code", O_RDONLY | O_NONBLOCK);
	if (codefile < 0) return nullptr;
	char codestr[16];
	while (true)
	{
		int r = read(codefile, codestr, 15);
		if (r > 0)
		{
			codestr[r] = '\0';
			int code = strtol(codestr, nullptr, 10);
			if (code > 0) break;
		}
		sleep(5);
	}
	close(codefile);
	dprintf(_log, "Got the code from " MECHAN_PIPE_DIR "/code\n");
	return _call(TDMAKE(checkAuthenticationCode)(codestr));
};

TDP(Object) Mechan::_processASWRegistration()
{
	dprintf(_log, "Processing authorizationStateWaitRegistration\n");
	return _call(TDMAKE(registerUser)(MECHAN_FIRST_NAME, MECHAN_SECOND_NAME));
};

TDP(Object) Mechan::_processASWPassword()
{
	dprintf(_log, "processing authorizationStateWaitPassword\n");
	return _call(TDMAKE(checkAuthenticationPassword)(MECHAN_PASSWORD));
};

void Mechan::_processUpdateAuthorizationState(TDP(updateAuthorizationState) updateauthorizationstate)
{
	switch (updateauthorizationstate->authorization_state_->get_id())
	{			
	case td::td_api::authorizationStateWaitTdlibParameters::ID:
		_processASWTdlibParameters();
		break;
		
	case td::td_api::authorizationStateWaitEncryptionKey::ID:
		_processASWEncryptionKey();
		break;
	
	case td::td_api::authorizationStateWaitPhoneNumber::ID:
		_processASWPhoneNumber();
		break;

	case td::td_api::authorizationStateWaitCode::ID:
		_processASWCode();
		break;
	
	case td::td_api::authorizationStateWaitRegistration::ID:
		_processASWRegistration();
		break;

	case td::td_api::authorizationStateWaitPassword::ID:
		_processASWPassword();
		break;

	case td::td_api::authorizationStateReady::ID:
		dprintf(_log, "Got authorization\n");
		_authorized = true;
		break;
	}
};

void Mechan::_processUpdateNewMessageText(TDP(message) message)
{
	//Opening pipe
	int writepipe = open(MECHAN_PIPE_DIR "/message", O_WRONLY | O_NONBLOCK);
	if (writepipe < 0) return;
	
	//Writing text to pipe
	std::string chatname = _getChatName(message->chat_id_);
	std::string username = _getUserName(message->sender_user_id_);
	TDP(messageText) messagetext = TDMOVEAS(messageText, message->content_);
	
	dprintf(writepipe, "!t %lli:%i:%s %i:%i:%s %i:%s\n",
		message->chat_id_, chatname.size(), chatname.c_str(),
		message->sender_user_id_, username.size(), username.c_str(),
		messagetext->text_->text_.size(), messagetext->text_->text_.c_str());
	
	//Closing pipe
	close(writepipe);
};

void Mechan::_processUpdateNewMessage(TDP(updateNewMessage) updatenewmessage)
{
	switch (updatenewmessage->message_->content_->get_id())
	{
	case td::td_api::messageText::ID:
		_processUpdateNewMessageText(TDMOVE(updatenewmessage->message_));
		break;
	}
};

void Mechan::_processMessage(TDP(Object) object)
{
	switch (object->get_id())
	{
	case td::td_api::updateAuthorizationState::ID:
		_processUpdateAuthorizationState(TDMOVEAS(updateAuthorizationState, object));
		break;	
		
	case td::td_api::updateNewMessage::ID:
		_processUpdateNewMessage(TDMOVEAS(updateNewMessage, object));
		break;
	}
};

unsigned int Mechan::_processMessages()
{
	unsigned int processed = 0;
	
	while (true)
	{
		td::Client::Response response;
	
		if (_messageQueue.size() == 0)
		{
			response = _client->receive(MECHAN_TIMEOUT);
		}
		else
		{
			response = std::move(_messageQueue[0]);
			_messageQueue.erase(_messageQueue.begin());
		}
	
		if (response.object != nullptr)
		{
			if (response.id == 0)
			{
				_processMessage(TDMOVE(response.object));
				processed++;
			}
		}
		else return processed;
	}
};

void Mechan::_processPipeMessageQ()
{
	_authorized = false;
};

void Mechan::_processPipeMessageT()
{
	//Reading delimiter
	char delim;
	if (read(_readpipe, &delim, 1) < 1 || delim != ' ') return;
		
	//Reding chat id
	std::int64_t chatid = _dreadnumber(_readpipe, ' ');
	if (chatid == 0) return;

	//Reading length
	unsigned int messagelen = _dreadnumber(_readpipe, ':');
	if (messagelen == 0) return;

	//Reading message
	std::string message(messagelen, '\0');
	if(read(_readpipe, &message[0], messagelen) < messagelen) return;
	
	//Sending message
	TDP(sendMessage) sendmessage = TDMAKE(sendMessage)();
	sendmessage->chat_id_ = chatid;
	sendmessage->reply_to_message_id_ = 0;
	sendmessage->options_ = TDMAKE(sendMessageOptions)();
	sendmessage->options_->disable_notification_ = false;
	sendmessage->options_->from_background_ = false;
	TDP(formattedText) formattedtext = TDMAKE(formattedText)();
	formattedtext->text_ = message;
	sendmessage->input_message_content_ = TDMAKE(inputMessageText)(TDMOVE(formattedtext), false, false);
	TDP(Object) result = _call(TDMOVE(sendmessage));
};

void Mechan::_processPipeMessage()
{
	char action;
	int r = read(_readpipe, &action, 1);
	if (r > 0)
	{
		switch(action)
		{
		case 'q':
			_processPipeMessageQ();
			break;
			
		case 't':
			_processPipeMessageT();
			break;
		}
	}
};

unsigned int Mechan::_processPipeMessages()
{
	unsigned int processed = 0;
	while (true)
	{
		char symbol;
		int r = read(_readpipe, &symbol, 1);
		if (r > 0)
		{
			if (symbol == '!')
			{
				_processPipeMessage();
				processed++;
			}
		}
		else return processed;
	}
};

void Mechan::loop()
{		
	while (_authorized)
	{
		_processPipeMessages();
		_processMessages();
		sleep(1);
	}
};

Mechan::~Mechan()
{
	if (_log >= 0) close(_log);
	if (_readpipe >= 0) close(_readpipe);
};

int _main()
{
	Mechan mechan;
	mechan.loop();
	return 0;
};

int main()
{
	return _main();
}
