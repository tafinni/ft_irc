#include "../../inc/Server.hpp"

void	Server::cmdUser(Client *client)
{

	if((client->getParams().size() < 2))
		return sendResponse(ERR_NOTENOUGHPARAM(client->getCommand()), client->getFd());

	if(client->getPassed() == false)
		return sendResponse(ERR_NOTREGISTERED(std::string("*")), client->getFd());

	else if (client->getUserName().size() > 0)
		return sendResponse(ERR_ALREADYREGISTERED(client->getNickName()), client->getFd());

	else
	{
		client->setUserName(client->getParams()[0]);
		client->setRealName(client->getMessage());
	}
	
	if (client->getPassed() && !client->getUserName().empty() && !client->getNickName().empty() && client->getNickName() != "*"  && !client->getNicked())
	{
		client->setNicked(true);
		sendResponse(RPL_CONNECTED(client->getNickName()), client->getFd());
	}
}
