
#include "../inc/Channel.hpp"

Channel::Channel()
{
	this->_invit_only = 0;
	this->_topicRestriction = false;
	this->_name = "";
	this->_topic_name = "";
	this->_limit = 0;
	char charaters[5] = {'i', 't', 'k', 'o', 'l'};
	for(int i = 0; i < 5; i++)
		_modes.push_back(std::make_pair(charaters[i],false));
}

Channel::Channel(Channel const &src)
{
	*this = src;
}

Channel &Channel::operator=(Channel const &src)
{
	if(this != &src)
	{
		this->_invit_only = src._invit_only;
		this->_topicRestriction = src._topicRestriction;
		this->created_at = src.created_at;
		this->_topic_name = src._topic_name;
		this->_clients = src._clients;
		this->_admins = src._admins;
		this->_name = src._name;
		this->_topic_name = src._topic_name;
		this->_password = src._password;
		this->_limit = src._limit;
		this->_modes = src._modes;
	}
	return *this;
}

Channel::~Channel()
{
	_clients.clear();
	_admins.clear();
	_modes.clear();
}


// get methods
std::string Channel::getName() { return this->_name; }
std::string Channel::getTopicName() { return this->_topic_name; }
std::string Channel::getPassword() { return this->_password; }
int Channel::getLimit() { return this->_limit; }
std::string Channel::getTimeCreation() { return this->_time_creation; }
std::string Channel::getCreatiOnTime() { return this->created_at; }
int Channel::getInvitOnly() { return this->_invit_only; }
bool Channel::getModeAtindex(size_t index) { return _modes[index].second; }
int Channel::GetClientsNumber() { return this->_clients.size() + this->_admins.size(); }
bool Channel::getTopicRestriction() const {return this->_topicRestriction;}
Client*Channel::get_client(int fd)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->getFd() == fd)
		{
			return &(*it);
		}
	}
	return NULL;
}

Client*Channel::get_admin(int fd)
{
	for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
	{
		if (it->getFd() == fd)
		{
			return &(*it);
		}
	}
	return NULL;
}

std::string Channel::getModes()
{
	std::string mode;
	for(size_t i = 0; i < _modes.size(); i++){
		if(_modes[i].first != 'o' && _modes[i].second)
			mode.push_back(_modes[i].first);
	}
	if(!mode.empty())
		mode.insert(mode.begin(), '+');
	return mode;
}



// Setters
void Channel::setName(std::string name_input) { this->_name = name_input; }
void Channel::setTopicName(std::string topic_name_input) { this->_topic_name = topic_name_input; }
void Channel::setPassword(std::string password_input) { this->_password = password_input; }
void Channel::setLimit(int limit_input) { this->_limit = limit_input; }
void Channel::setTimeCreation(std::string time_creation_input) { this->_time_creation = time_creation_input; }
void Channel::setModeAtindex(size_t index, bool mode) { _modes[index].second = mode; }
void Channel::setInvitOnly(int invit_only) { this->_invit_only = invit_only; }
void Channel::setTopicRestriction(bool value) { this->_topicRestriction = value; }
void Channel::setCreatedAt()
{
	std::time_t _time = std::time(NULL);
	std::ostringstream oss;
	oss << _time;
	this->created_at = std::string(oss.str());
}


// Methods

bool Channel::isClientInChannel(std::string &nick)
{
	for(size_t i = 0; i < _clients.size(); i++){
		if(_clients[i].getNickName() == nick)
			return true;
	}
	for(size_t i = 0; i < _admins.size(); i++){
		if(_admins[i].getNickName() == nick)
			return true;
	}
	return false;
}


Client* Channel::findClientInChannel(const std::string& name) 
{
	for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
	{
		if (it->getNickName() == name)
		{
			return &(*it);
		}
	}

	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->getNickName() == name)
		{
			return &(*it);
		}
	}
	return NULL;
}


void Channel::addClient(Client newClient) {	_clients.push_back(newClient); }
void Channel::addAdmin(Client newClient) { 	_admins.push_back(newClient); }

void Channel::removeClient(int fd)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (fd == it->getFd())
		{
			_clients.erase(it);
			break;
		}
	}
}

void Channel::removeAdmin(int fd)
{
	for (std::vector<Client>::iterator it = _admins.begin(); it != _admins.end(); ++it)
	{
		if (fd == it->getFd())
		{
			_admins.erase(it);
			break;
		}
	}
}

std::string Channel::clientChannel_list()
{
	std::string list;
	for(size_t i = 0; i < _admins.size(); i++)
	{
		list += "@" + _admins[i].getNickName();
		if((i + 1) < _admins.size())
			list += " ";
	}
	if(_clients.size())
		list += " ";
	for(size_t i = 0; i < _clients.size(); i++)
	{
		list += _clients[i].getNickName();
		if((i + 1) < _clients.size())
			list += " ";
	}
	return list;
}

bool Channel::changeClientToAdmin(std::string& nick)
{
	size_t i = 0;
	for(; i < _clients.size(); i++){
		if(_clients[i].getNickName() == nick)
			break;
	}
	if(i < _clients.size()){
		_admins.push_back(_clients[i]);
		_clients.erase(i + _clients.begin());
		return true;
	}
	return false;
}

bool Channel::changeAdminToClient(std::string& nick)
{
	size_t i = 0;
	for(; i < _admins.size(); i++){
		if(_admins[i].getNickName() == nick)
			break;
	}
	if(i < _admins.size()){
		_clients.push_back(_admins[i]);
		_admins.erase(i + _admins.begin());
		return true;
	}
	return false;

}

// send to all methods  //added \r\n or not
void Channel::sendToAll(std::string rpl1)
{
	for(size_t i = 0; i < _admins.size(); i++)
		if(send(_admins[i].getFd(), rpl1.c_str(), rpl1.size(),0) == -1)
			std::cerr << "send() faild" << std::endl;
	for(size_t i = 0; i < _clients.size(); i++)
		if(send(_clients[i].getFd(), rpl1.c_str(), rpl1.size(),0) == -1)
			std::cerr << "send() faild" << std::endl;
}

void Channel::sendToAll(std::string rpl1, int fd)
{
	for(size_t i = 0; i < _admins.size(); i++)
	{
		if(_admins[i].getFd() != fd)
			if(send(_admins[i].getFd(), rpl1.c_str(), rpl1.size(),0) == -1)
				std::cerr << "send() faild" << std::endl;
	}
	for(size_t i = 0; i < _clients.size(); i++)
	{
		if(_clients[i].getFd() != fd)
			if(send(_clients[i].getFd(), rpl1.c_str(), rpl1.size(),0) == -1)
				std::cerr << "send() faild" << std::endl;
	}
}
