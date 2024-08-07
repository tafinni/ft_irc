#include "../../inc/Server.hpp"


bool Server::changeNick(std::string& oldnick, std::string& newnick)
{
	bool sent = false;

	for (std::vector<Client>::iterator cli = _clients.begin(); cli != _clients.end(); cli++)
	{
		for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); it++)
		{
			Client *client = it->findClientInChannel(oldnick);
			Client *other = it->findClientInChannel(cli->getNickName());
			if (client)
			{
				client->setNickName(newnick);
				if (sent == false)
					sendResponse(RPL_NICKCHANGE(oldnick, newnick), client->getFd());
				sent = true;
			}
			if (client && other)
			{
				sendResponse(RPL_NICKCHANGE(oldnick, newnick), other->getFd());
				break ;
			}
		}
	}
	return sent;
}

void Server::cmdNick(Client *client)
{
	std::string inuse;
	if(client->getParams().size() < 1)
		return sendResponse(ERR_NOTENOUGHPARAM(client->getCommand()), client->getFd());

	// get list of clients and make sure nickname hasn't been taken
	std::vector<Client> clients = Server::getClients();
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); it++)
		if (it->getNickName() == client->getParams()[0])
	    	return sendResponse(ERR_NICKINUSE(std::string(client->getParams()[0])), client->getFd()); 

	// Check nickname max lenght
	if (client->getParams()[0].size() > 9)
		return sendResponse(ERR_ERRONEUSNICK(std::string(client->getParams()[0])), client->getFd());

	// check for invalid first character
	if (std::isalpha(client->getParams()[0][0]) == false && (client->getParams()[0][0] != '_' && client->getParams()[0][0] != '-' && client->getParams()[0][0] != '|' && client->getParams()[0][0] != '\\' && client->getParams()[0][0] != '[' && client->getParams()[0][0] != ']' && client->getParams()[0][0] != '{' && client->getParams()[0][0] != '}'))
		return sendResponse(ERR_ERRONEUSNICK(std::string(client->getParams()[0])), client->getFd());

	// check for invalid characters
	for (char c : client->getParams()[0])
		if (!std::isalnum(c) && (c != '_' && c != '-' && c != '|' && c != '\\' && c != '[' && c != ']' && c != '{' && c != '}'))
			return sendResponse(ERR_ERRONEUSNICK(std::string(client->getParams()[0])), client->getFd());


	if (client->getPassed() == true)
	{
		std::string oldnick = client->getNickName();
		client->setNickName(client->getParams()[0]);
		if(!oldnick.empty() && oldnick != client->getParams()[0])
		{
			if(oldnick == "*" && !client->getUserName().empty())
			{
				client->setNicked(true);
				sendResponse(RPL_CONNECTED(client->getNickName()), client->getFd());
				sendResponse(RPL_NICKCHANGE(client->getNickName(),client->getCommand()), client->getFd());
			}
			else

			{
				if (changeNick(oldnick, client->getParams()[0]) == false)
					sendResponse(RPL_NICKCHANGE(oldnick,client->getParams()[0]), client->getFd());
			}

			return;
		}
	}
	else if (client->getPassed() == false)
		sendResponse(ERR_NOTREGISTERED(client->getCommand()), client->getFd());


	if(client->getPassed() && !client->getUserName().empty() && !client->getNickName().empty() && client->getNickName() != "*" && !client->getNicked())
	{
		client->setNicked(true);
		sendResponse(RPL_CONNECTED(client->getNickName()), client->getFd());
	}
}
