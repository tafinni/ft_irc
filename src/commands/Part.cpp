#include "../../inc/Server.hpp"


void Server::cmdPart(Client *client)
{
	if (client->getParams().size() < 1)
		return sendResponse(ERR_NOTENOUGHPARAM(client->getCommand()), client->getFd());

	// loop all channels to part from
	for (std::vector<std::string>::iterator it = client->getParams().begin(); it != client->getParams().end(); it++)
	{	
		// split parameters in case of commas
		std::vector<std::string> splitted = splitParams(*it);
		for (std::vector<std::string>::iterator sp = splitted.begin(); sp != splitted.end(); sp++)
		{
			std::string channel_name;
			bool found = false;

			// loop all channels for match for parameter
			for (size_t i = 0; i < channels.size(); i++)
			{
				// remove # from channel name
				std::string sp_name = *sp;
				if (sp_name[0] == '#')
					sp_name.erase(0, 1);
				channel_name = sp_name;
				
				// find correct channel
				if (channels[i].getName() == sp_name)
				{
					found = true;

					if (!channels[i].findClientInChannel(client->getNickName()))
					{
						sendResponse(ERR_NOTONCHANNEL(client->getNickName(), channels[i].getName()), client->getFd());
						break ;
					}

					// send PART message to clients in channel
					if (client->getMessage().size() > 0)
						channels[i].sendToAll(":" + client->getNickName() + "!~" + client->getUserName() + "@localhost PART #" + channels[i].getName() + " :" + client->getMessage() + CRLF);
					else
						channels[i].sendToAll(":" + client->getNickName() + "!~" + client->getUserName() + "@localhost PART #" + channels[i].getName() + CRLF);

					// remove client from channel
					channels[i].removeAdmin(client->getFd());
					channels[i].removeClient(client->getFd());

					// if no one is left on channel, remove channel
					if (channels[i].GetClientsNumber() < 1)
						channels.erase(channels.begin() + i);
					
					break ;
				}
			}
			if (found == false)
				sendResponse(ERR_NOSUCHCHANNEL(client->getNickName(), channel_name), client->getFd());
		}
	}
}

