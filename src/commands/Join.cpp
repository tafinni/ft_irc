#include "../../inc/Server.hpp"


/*
 Command: JOIN
   Parameters: <channel>{,<channel>} [<key>{,<key>}]

   The JOIN command is used by client to start listening a specific
   channel. 

	ERR_TOOMANYTARGETS (407)  if more than 10 Channels

	ERR_NEEDMOREPARAMS (461) if there are not enough parameters            
	ERR_INVITEONLYCHAN  (473) "<channel> :Cannot join channel (+i)"            
	ERR_BADCHANNELKEY (475) "<channel> :Cannot join channel (+k)"
	ERR_CHANNELISFULL (471) "<channel> :Cannot join channel (+l)"             
	ERR_NOSUCHCHANNEL (403)   Used to indicate the given channel name is invalid.             
	ERR_TOOMANYCHANNELS (405) Sent to a user when they have joined the maximum
							  number of allowed channels and they try to join
							  another channel.

*/


int Server::splitJoin(std::vector<std::pair<std::string, std::string> > &token, std::vector<std::string> _params, int _fd)
{
	if (_params.empty()) 
	{ 
		token.clear(); 
		return 0; 
	}

	std::string ChStr = _params[0];
	std::string PassStr;
	if (_params.size() > 1)
	{
		PassStr = _params[1];
	}

	// Split channel names by commas
	std::string buff;
	for (size_t i = 0; i < ChStr.size(); i++) {
		if (ChStr[i] == ',') 
		{
			if (!buff.empty()) 
			{
				token.push_back(std::make_pair(buff, ""));
				buff.clear();
			}
		} 
		else
		{
			buff += ChStr[i];
		}
	}
	if (!buff.empty())
	{
		token.push_back(std::make_pair(buff, ""));
	}

	// Split passwords by commas and associate them with channels
	if (!PassStr.empty()) {
		size_t j = 0;
		buff.clear();
		for (size_t i = 0; i < PassStr.size(); i++) {
			if (PassStr[i] == ',') {
				if (j < token.size()) {
					token[j].second = buff;
					j++;
				}
				buff.clear();
			} else {
				buff += PassStr[i];
			}
		}
		if (j < token.size()) {
			token[j].second = buff;
		}
	}

	// Erase empty channel names
	for (size_t i = 0; i < token.size(); i++) 
	{
		if (token[i].first.empty())
		{
			token.erase(token.begin() + i--);
		}
	}

	// Check for invalid channels and strip '#' from valid channels
	for (size_t i = 0; i < token.size(); i++) 
	{
		if (token[i].first[0] != '#') 
		{
			sendErrorToClient(403, getClient(_fd)->getNickName(), token[i].first, getClient(_fd)->getFd(), " :No such channel\r\n");
			token.erase(token.begin() + i--);
		}
		else 
		{
			token[i].first.erase(token[i].first.begin());
		}
	}
	return 1;
}


int Server::searchForClients(std::string nickname)
{
	int count = 0;
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i].findClientInChannel(nickname))
		{
			count++;
		}
	}
	return count;
}


bool isInvited(Client *cli, std::string ChName, int flag)
{
	if(cli->getInviteChannel(ChName))
	{
		if (flag == 1)
		{
			cli->rmChannelInvite(ChName);
		}
		return true;
	}
	return false;
}


void Server::existChannel(std::vector<std::pair<std::string, std::string> >&token, size_t i, size_t j, int _fd)
{
	Client *cli = getClient(_fd);

	// check if client in Channel already
	if (this->channels[j].findClientInChannel(cli->getNickName()))
		return;
	
	// if client in more than 10 channels
	if (searchForClients(cli->getNickName()) >= 10)
		{sendErrorToClient(405, cli->getNickName(), cli->getFd(), " :You have joined too many channels\r\n"); return;}

	// check password
	if (!this->channels[j].getPassword().empty() && this->channels[j].getPassword() != token[i].second){
		if (!isInvited(cli, token[i].first, 0))
			{sendErrorToClient(475, cli->getNickName(), "#" + token[i].first, cli->getFd(), " :Cannot join channel (+k) - bad key\r\n"); return;}
	}

	// If the channel has the "invite only" mode set, it checks whether the user is invited to the channel. If not, an error is sent, and the execution of the function is halted.
	if (this->channels[j].getInvitOnly())
	{
		if (!isInvited(cli, token[i].first, 1))
		{
			sendErrorToClient(473, cli->getNickName(), "#" + token[i].first, cli->getFd(), " :Cannot join channel (+i)\r\n"); 
			return;
		}
	}

	// check for limits 
	if (this->channels[j].getLimit() && this->channels[j].GetClientsNumber() >= this->channels[j].getLimit())
	{
		sendErrorToClient(471, cli->getNickName(), "#" + token[i].first, cli->getFd(), " :Cannot join channel (+l)\r\n"); 
		return;
	}

	// add clients
	this->channels[j].addClient(*cli);


	if(channels[j].getTopicName().empty())
		sendResponse(RPL_JOINMSG(cli->getHostName(),cli->getIp(),token[i].first) + \
			RPL_NAMREPLY(cli->getNickName(),channels[j].getName(),channels[j].clientChannel_list()) + \
			RPL_ENDOFNAMES(cli->getNickName(),channels[j].getName()),_fd);
	else
		sendResponse(RPL_JOINMSG(cli->getHostName(),cli->getIp(),token[i].first) + \
			RPL_TOPICIS(cli->getNickName(),channels[j].getName(),channels[j].getTopicName()) + \
			RPL_NAMREPLY(cli->getNickName(),channels[j].getName(),channels[j].clientChannel_list()) + \
			RPL_ENDOFNAMES(cli->getNickName(),channels[j].getName()),_fd);
	channels[j].sendToAll(RPL_JOINMSG(cli->getHostName(),cli->getIp(),token[i].first), _fd);


}
void Server::notExistChannel(std::vector<std::pair<std::string, std::string> >&token, size_t i, int _fd)
{
	Client *cli = getClient(_fd);

	if (searchForClients(cli->getNickName()) >= 10)
	{
		sendErrorToClient(405, cli->getNickName(), cli->getFd(), " :You have joined too many channels\r\n"); 
		return;
	}

	Channel newChannel;
	newChannel.setName(token[i].first);
	newChannel.addAdmin(*cli);
	newChannel.setCreatedAt();
	this->channels.push_back(newChannel);
	// notifiy then the client joined the channel
	sendResponse(RPL_JOINMSG(cli->getHostName(),cli->getIp(),newChannel.getName()) + \
		RPL_NAMREPLY(cli->getNickName(),newChannel.getName(),newChannel.clientChannel_list()) + \
		RPL_ENDOFNAMES(cli->getNickName(),newChannel.getName()),_fd);
}

void Server::cmdJoin(std::vector<std::string> _params, int _fd)
{

	if (_params.size() < 1)
    {
        sendErrorToClient(461, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :Not enough parameters\r\n"); 
        return;
    }
	std::vector<std::pair<std::string, std::string> > token;
	if (!splitJoin(token, _params, _fd))
	{
		sendErrorToClient(461, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :Not enough parameters\r\n"); 
		return;
	}

	// Checking the number of channels to connect
	if (token.size() > 10)
	{
		sendErrorToClient(407, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :Too many channels\r\n"); 
		return;
	}


	for (size_t i = 0; i < token.size(); i++)
	{
		bool isChannelExists = false;
		for (size_t j = 0; j < this->channels.size(); j++)
		{
			if (this->channels[j].getName() == token[i].first)
			{
				existChannel(token, i, j, _fd);
				isChannelExists = true;
				break;
			}
		}
		if (!isChannelExists)
		{
			notExistChannel(token, i, _fd);
		}
	}
}
