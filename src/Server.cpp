#include "../inc/Server.hpp"

std::vector<Client> Server::_clients;
std::vector<pollfd> Server::_pollfds;
std::vector<Channel> Server::channels;

Server::Server() {}

Server::Server(int port, std::string pwd): _port(port), _password(pwd) {this->_socketfd = -1;};
Server::Server(Server const &src){*this = src;}
Server &Server::operator=(Server const &src)
{
	if (this != &src){
		this->_port = src._port;
		this->_socketfd = src._socketfd;
		this->_clients = src._clients;
		this->channels = src.channels;
		this->_pollfds = src._pollfds;
	}
	return *this;
}

Server::~Server() 
{
	_clients.clear();
	_pollfds.clear();
	channels.clear();
}

void Server::start()
{
	setServerSocket();

	std::cout << "Waiting to accept a connection...\n";
	while (1)
	{
		try
		{
			if(poll(&_pollfds[0],_pollfds.size(),-1) == -1)
				throw std::runtime_error("poll() faild");
			
			for (size_t i = 0; i < _pollfds.size(); i++)
			{
				// check if there is data to read
				if (_pollfds[i].revents & POLLIN) 
				{
					if (_pollfds[i].fd == _socketfd)
						this->newConnection();
					else
						this->receiveMessage(_pollfds[i].fd);
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	// close the fd's when the server gets signal and breaks the loop
	close_fds();
}

void	Server::close_fds()
{
	for(size_t i = 0; i < _clients.size(); i++)
	{
		std::cout << RED << "Client <" << _clients[i].getFd() << "> Disconnected" << WHI << std::endl;
		close(_clients[i].getFd());
	}
	if (_socketfd != -1)
	{	
		std::cout << RED << "Server <" << _socketfd << "> Disconnected" << WHI << std::endl;
		close(_socketfd);
	}
}

void Server::setServerSocket()
{

	int en = 1;
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = INADDR_ANY;
	add.sin_port = htons(_port);
	_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(_socketfd == -1)
		throw(std::runtime_error("faild to create socket"));
	if(setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	 if (fcntl(_socketfd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(_socketfd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_socketfd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));
	new_cli.fd = _socketfd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	_pollfds.push_back(new_cli);

}

void Server::newConnection()
{
	Client cli;
	memset(&cliadd, 0, sizeof(cliadd));
	socklen_t len = sizeof(cliadd);
	int incofd = accept(_socketfd, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
		{std::cout << "fcntl() failed" << std::endl; return;}
	new_cli.fd = incofd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	cli.setFd(incofd);
	cli.setIp(inet_ntoa((cliadd.sin_addr)));

	if (_clients.size() == 20)
	{
		sendResponse("Maximum amount of clients reached\r\n", cli.getFd());
		close(cli.getFd());
		return ;
	}

	_clients.push_back(cli);
	_pollfds.push_back(new_cli);
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::receiveMessage(const int fd)
{
	std::vector<std::string> cmd;
	// buffer for the received data
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client *client = getClient(fd);
	// receive the data
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	// check if the client disconnected
	if(bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		rmChannels(fd);
		removeClient(fd);
		removeFds(fd);
		close(fd);
	}
	else
	{ 
		client->setBuffer(buff);

		const char* substr = "\r\n";
		char* pos = std::strstr(buff, substr);
		if (pos == nullptr)
		{	
			if (client->getBuffer().size() == 512)
				client->getBuffer().clear();
			throw Client::BackToLoop();
		}	
	 	
		cmd = saveMsg(client->getBuffer());

		for(size_t i = 0; i < cmd.size(); i++)
			this->executeCommand(cmd[i], fd);
		if(getClient(fd))
			getClient(fd)->clearBuffer();
	}
}

bool Server::isRegistered(Client *client)
{
	if (client->getNickName().empty() || client->getUserName().empty() || client->getNickName() == "*"  || !client->getNicked())
		return false;
	return true;
}

void Server::executeCommand(std::string &cmd, int fd)
{
	if(cmd.empty())
		return ;

	if(!getClient(fd))
		return ;
	Client *client = getClient(fd);
	if (client) {
		client->parseMsg(cmd); 
	}

	if(client->getCommand() == "PASS" || client->getCommand() == "pass")
		cmdPass(client);
	else if (client->getCommand() == "NICK" || client->getCommand() == "nick")
		cmdNick(client);
	else if(client->getCommand() == "USER" || client->getCommand() == "user")
		cmdUser(client);
	else if (client->getCommand() == "QUIT" || client->getCommand() == "quit")
		cmdQuit(client);

	else if(isRegistered(client) == true)
	{	
		if (client->getCommand() == "PART" || client->getCommand() == "part")
			cmdPart(client);	
		else if (client->getCommand() == "PRIVMSG" || client->getCommand() == "privmsg")
			cmdPrivmsg(client->getParams(), client->getMessage(), client->getFd());
		else if (client->getCommand() == "JOIN" || client->getCommand() == "join")
		 	cmdJoin(client->getParams(), client->getFd());
		else if (client->getCommand() == "KICK" || client->getCommand() == "kick")
			cmdKick(client->getParams(), client->getMessage(), client->getFd());
		else if (client->getCommand() == "INVITE" || client->getCommand() == "invite")
			cmdInvite(client->getParams(), client->getFd());
		else if (client->getCommand() == "TOPIC" || client->getCommand() == "topic")
			cmdTopic(client->getParams(), client->getMessage(), client->getFd());
		else if (client->getCommand() == "MODE" || client->getCommand() == "mode")
			cmdMode(client->getParams(), client->getMessage(), client->getFd());
		else if (client->getCommand() == "PING" || client->getCommand() == "WHO")
			return ;
		else
			sendResponse(ERR_CMDNOTFOUND(getClient(fd)->getNickName(),cmd),fd);
	} 
	else if (client->getCommand() == "PASS" || client->getCommand() == "registpass" || client->getCommand() == "USER" || client->getCommand() == "user")
		sendErrorToClient(462, client->getNickName(), client->getFd(), ":You may  not reregister");
	else
		sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
	
}


std::vector<std::string> Server::saveMsg(std::string str)

{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	while(std::getline(stm, line))
	{
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
} 

// get

std::vector<Client> Server::getClients() {return _clients;}

Channel *Server::getChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].getName() == name)
			return &channels[i];
	}
	return NULL;
}

Client *Server::getClient(int fd)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].getFd() == fd)
			return &this->_clients[i];
	}
	return NULL;
}

Client *Server::getClientByNickname(std::string nickname)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].getNickName() == nickname)
			return &this->_clients[i];
	}
	return NULL;
}


// print responses

void Server::sendErrorToClient(int code, std::string nick_name, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << nick_name << msg;

	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(), 0) == -1)
		std::cerr << "send() faild" << std::endl;
}


void Server::sendErrorToClient(int code, std::string nick_name, std::string channelname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << nick_name << " " << channelname << msg;

	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(), 0) == -1)
		std::cerr << "send() faild" << std::endl;
}


void Server::sendResponse(std::string response, int fd)
{
	std::cout << "Response:\n" << response;
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}


// close and removes
void Server::closeClient(Client *client)
{
	std::cout << "Client " << client->getFd() << " disconnected." << std::endl;

	// checks channels for client
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].findClientInChannel(client->getNickName()))
		{
			// if channel has no other clients, erases channel
			if (channels[i].GetClientsNumber() == 1)
				channels.erase(channels.begin() + i);
			else
			{
				// removes client from channel
				channels[i].removeClient(client->getFd());
				channels[i].removeAdmin(client->getFd());

				// creates message to send all channel participants
				if (client->getMessage().size() > 0)
					channels[i].sendToAll(":" + client->getNickName() + "!" + client->getUserName() + "@" + client->getIp() + " " + client->getCommand() + " :" + client->getMessage() + "\n");
				else
					channels[i].sendToAll(":" + client->getNickName() + "!" + client->getUserName() + "@" + client->getIp() + " " + client->getCommand() + "\n");
			}
		}
	}

	// remove from pollfds
	for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); it++)
	{
		if (it->fd == client->getFd())
		{
			_pollfds.erase(it);
			break ;
		}
	}

	close(client->getFd());

	// remove from server clients
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->getNickName() == client->getNickName())
		{
			_clients.erase(it);
			break ;
		}
	}
}

void	Server::rmChannels(int fd){
	for (size_t i = 0; i < this->channels.size(); i++){
		int flag = 0;
		if (channels[i].get_client(fd))
			{channels[i].removeClient(fd); flag = 1;}
		else if (channels[i].get_admin(fd))
			{channels[i].removeAdmin(fd); flag = 1;}
		if (channels[i].GetClientsNumber() == 0)
			{channels.erase(channels.begin() + i); i--; continue;}
		if (flag){
			std::string rpl = ":" + getClient(fd)->getNickName() + "!~" + getClient(fd)->getUserName() + "@localhost QUIT Quit\r\n";
			channels[i].sendToAll(rpl);
		}
	}
}

void Server::removeClient(int fd){
	for (size_t i = 0; i < this->_clients.size(); i++){
		if (this->_clients[i].getFd() == fd)
			{this->_clients.erase(this->_clients.begin() + i); return;}
	}
}

void Server::removeFds(int fd){
	for (size_t i = 0; i < this->_pollfds.size(); i++){
		if (this->_pollfds[i].fd == fd)
			{this->_pollfds.erase(this->_pollfds.begin() + i); return;}
	}
}
