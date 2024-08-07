#include "../../inc/Server.hpp"

void Server::cmdPass(Client *client)
{
	if (client->getPassed() == true)
		return sendResponse(ERR_ALREADYREGISTERED(client->getNickName()), client->getFd());

	if (client->getParams().size() < 1)
		return sendResponse(ERR_NOTENOUGHPARAM(client->getCommand()), client->getFd());

	std::string given_pwd;
	for (std::vector<std::string>::iterator it = client->getParams().begin(); it != client->getParams().end(); it++)
		given_pwd.append(*it);

	if (given_pwd == _password)
		client->setPassed(true);
	else
        sendResponse(ERR_INCORPASS(std::string("*")), client->getFd());
}