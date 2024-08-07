#include "../../inc/Server.hpp"

void Server::cmdQuit(Client *client)
{
	// Check if QUIT is used correctly
	// If QUIT has parameters it shouldn't work
	if (client->getParams().size() != 0)
		sendErrorToClient(461, client->getNickName(), client->getFd(), client->getCommand() + " :Unknown command");

	closeClient(client);
}
