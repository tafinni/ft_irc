#include "../../inc/Server.hpp"

/*
 Command: TOPIC
   Parameters: <channel> [<topic>]

   The TOPIC message is used to change or view the topic of a channel.
   The topic for channel <channel> is returned if there is no <topic>
   given.  If the <topic> parameter is present, the topic for that
   channel will be changed, if the channel modes permit this action.

   Numeric Replies:

	ERR_NEEDMOREPARAMS (461)  if there are not enough parameters
	ERR_NOSUCHCHANNEL (403) if the given channel does not exist
	ERR_NOTONCHANNEL (442)  if the client is not on the channel
	ERR_CHANOPRIVSNEEDED (482)  if the client is not a channel operator
	RPL_NOTOPIC (331)   if no topic is set
	RPL_TOPIC (332) if the topic is set
	RPL_TOPICWHOTIME (333) if the topic is set

:localhost 442 #jj :You're not on that channel
:localhost 442 jj :You're Not a channel operator
*/

std::string Server::tTopic()
{
	std::time_t current = std::time(NULL);
	std::stringstream res;
	res << current;
	return res.str();
}

void Server::cmdTopic(const std::vector<std::string>& _params, const std::string& _message, const int &_fd)
{

    Client* client = getClient(_fd);
    if (!client) {
        return;
    }

	if (_params.empty())
	{
		sendErrorToClient(461, client->getNickName(), _fd, " :Not enough parameters\r\n");
		return;
	}

	std::string nmch = _params[0].substr(1);
	Channel* ch = getChannel(nmch);
	if (!ch) 
	{
		sendErrorToClient(403, "#" + nmch, _fd, " :No such channel\r\n");
		return;
	}

	if (!(ch->get_client(_fd)) && !(ch->get_admin(_fd)))
	{
		sendErrorToClient(442, "#" + nmch, _fd, " :You're not on that channel\r\n"); 
		return;
	}

	// TOPIC #channel
	if (_params.size() == 1 && _message.empty() && !client->getTrailingFlag()) 
	{
		if (ch->getTopicName().empty())
		{
			sendResponse(": 331 " + client->getNickName() + " " + "#" + nmch + " :No topic is set\r\n", _fd);
		}
		else
		{
			sendResponse(": 332 " + client->getNickName() + " " + "#" + nmch + " " + ch->getTopicName() + "\r\n", _fd); 
			sendResponse(": 333 " + client->getNickName() + " " + "#" + nmch + " " + client->getNickName() + " " + ch->getTimeCreation() + "\r\n", _fd); 
		}
		return;
	}

	// TOPIC #channel :
	if (_params.size() == 1 && _message.empty() && client->getTrailingFlag()) 
	{

		if (ch->getTopicRestriction() && !ch->get_admin(_fd))
		{
			sendErrorToClient(482, "#" + nmch, _fd, " :You're Not a channel operator\r\n"); 
			return;
		}

		// Delete the current topic
		ch->setTopicName("");
		std::string rpl = ":" + client->getNickName() + "!" + client->getUserName() + "@localhost TOPIC #" + nmch + " :\r\n"; 
		ch->sendToAll(rpl);
		return;
	}

	//TOPIC #channel :new topic
	if (_params.size() >= 1 && !_message.empty()) 
	{

		std::string new_topic = _message;

		if (ch->getTopicRestriction() && !ch->get_admin(_fd))
		{
			sendErrorToClient(482, "#" + nmch, _fd, " :You're Not a channel operator\r\n"); 
			return;
		}

		ch->setTimeCreation(tTopic());
		ch->setTopicName(new_topic);

		std::string rpl = ":" + client->getNickName() + "!" + client->getUserName() + "@localhost TOPIC #" + nmch + " " + new_topic + "\r\n"; 
		ch->sendToAll(rpl);
	}
}
