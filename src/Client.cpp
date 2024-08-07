#include "../inc/Client.hpp"

Client::Client(): _registered(false), _passed(false), _nicked(false) {}

Client::Client(Client const &src) {*this = src;}

Client &Client::operator=(Client const &src)
{
	if (this != &src)
	{
		this->_nickname = src._nickname;
		this->_username = src._username;
		this->_realname = src._realname;
		this->_ip = src._ip;
		this->_fd = src._fd;
		this->_registered = src._registered;
		this->_passed = src._passed;
		this->_nicked = src._nicked;
		this->_buffer = src._buffer;
		this->_trailing_flag = src._trailing_flag;
		this->_command = src._command;
		this->_params = src._params;
		this->_message = src._message;
		this->_channelsInvite = src._channelsInvite;
	}
	return *this;
}

Client::~Client() 
{
	_params.clear();
	_channelsInvite.clear();
}

void Client::clearMessageData()
{
	_command.clear();
	_message.clear();
	_params.clear();
}

void Client::parseMsg(const std::string &cmd)
{
	setTrailingFlag(false);
	// Clear previously parsed message
	clearMessageData();
	_buffer.clear();


	// Split the command into parts
	std::vector<std::string> list;
	std::istringstream iss(cmd);
	std::string word;
	while (iss >> word) {
		list.push_back(word);
	}

	// Checks if there was no input
	if (list.empty())
		return;
	// Checks if command has something else than alphabet
	for (size_t i = 0; i < list[0].size(); i++)
	{
		if (!std::isalpha(list[0][i]))
		{
			Server::sendResponse(ERR_CMDNOTFOUND(_nickname,list[0]),_fd);
			Client::BackToLoop();
		}
	}
	_command = list[0];

	// Checks if message has only command
	if (list.size() == 1)
		return ;


	// Finding parameters and adding them to std::vector
	std::vector<std::string>::iterator start;
	for (std::vector<std::string>::iterator it = list.begin() + 1; it != list.end(); it++)
	{
		// Message starts with :, check that it is at the beginning of string
		size_t pos = it->find(':');
		if (pos != std::string::npos)
		{
			setTrailingFlag(true);
			if (pos != 0)
			{
				Server::sendResponse(ERR_CMDNOTFOUND(_nickname,list[pos]),_fd);
				Client::BackToLoop();
			}
			start = it;

			break;
		}
		_params.push_back(*it);

		// Checks if there are words left and returns if everything is iterated
		if (it + 1 == list.end())
			return;
	}

	// Turns message part into one string
	for (std::vector<std::string>::iterator it = start; it != list.end(); it++)
	{
		_message.append(*it);
		if (it + 1 != list.end())
			_message.append(" ");
	}
	_message = _message.substr(1, _message.size() - 1);
}

void Client::addChannelInvite(std::string &chname) {_channelsInvite.push_back(chname);}

void Client::rmChannelInvite(std::string &chname)
{
	for (size_t i = 0; i < this->_channelsInvite.size(); i++)
	{
		if (this->_channelsInvite[i] == chname)
		{
			this->_channelsInvite.erase(this->_channelsInvite.begin() + i);
			return;
		}
	}
}

const char* Client::BackToLoop::what() const throw() { return nullptr;}

void Client::clearBuffer(){_buffer.clear();}
void Client::addToBuffer(char c) {_buffer += c;}

/* Set */
void Client::setUserName(std::string& username){this->_username = username;}
void Client::setNickName(std::string& nickName){this->_nickname = nickName;}
void Client::setTrailingFlag(bool flag) { _trailing_flag = flag; }
void Client::setRegistered(bool value){_registered = value;}
void Client::setFd(int fd) { this->_fd = fd;}
void Client::setIp(std::string IPaddr) { this->_ip = IPaddr;}
void Client::setBuffer(std::string buff) { this->_buffer += buff;}
void Client::setRealName(std::string& realname) {_realname = realname;}
void Client::setPassed(bool pass) {_passed = pass;}
void Client::setNicked(bool nick) {_nicked = nick;}

/* Get */
bool Client::getTrailingFlag() const { return _trailing_flag; }
std::string Client::getNickName(){return this->_nickname;}
std::string Client::getUserName(){return this->_username;}
std::string Client::getCommand() const {return _command;}
std::vector<std::string>& Client::getParams() {return _params;}
std::string& Client::getMessage() {return _message;}
bool Client::getPassed() {return _passed;}
bool Client::getNicked() {return _nicked;}
const std::string& Client::getIp() const {return _ip;}
const int& Client::getFd() const {return _fd;}
const bool& Client::getRegistered() const {return _registered;}
std::string& Client::getBuffer() {return _buffer;}

std::string Client::getHostName()
{
	std::string hostname = this->getNickName() + "!" + this->getUserName();
	return hostname;
}

bool Client::getInviteChannel(std::string &ChName)
{
	for (size_t i = 0; i < this->_channelsInvite.size(); i++){
		if (this->_channelsInvite[i] == ChName)
			return true;
	}
	return false;
}



