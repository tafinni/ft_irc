#include "../../inc/Server.hpp"

/*
   Command: KICK
   Parameters: <channel> <user> [<comment>]

   The KICK command can be  used  to  forcibly  remove  a  user  from  a
   channel.  
   

	ERR_NEEDMOREPARAMS  (461)  if there are not enough parameters       
	ERR_NOSUCHCHANNEL (403) if the given channel does not exist            
	ERR_CHANOPRIVSNEEDED (482)  if the client is not a channel operator
	ERR_NOTONCHANNEL (442)  if the client is not on the channel

*/

std::vector<std::string> Server::splitByComma(const std::string& str)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, ','))
	{
		if (!item.empty() && item[0] == '#')
		{
			item.erase(0, 1); // Remove leading '#'
		}
		if (!item.empty())
		{
			result.push_back(item);
		}
	}
	return result;
}

void Server::kickUserFromChannel(const std::string& channel_name, const std::string& user_name, const std::string& reason, const int& _fd)
{
	Channel *ch = getChannel(channel_name);
	if (!ch)
	{
		sendErrorToClient(403, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :No such channel\r\n");
		return;
	}

	if (!ch->get_client(_fd) && !ch->get_admin(_fd))
	{
		sendErrorToClient(442, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :You're not on that channel\r\n");
		return;
	}

	if (!ch->get_admin(_fd))
	{
		sendErrorToClient(482, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :You're not channel operator\r\n");
		return;
	}

	if (ch->findClientInChannel(user_name))
	{
		std::stringstream ss;
		ss << ":" << getClient(_fd)->getNickName() << "!~" << getClient(_fd)->getUserName() << "@" << "localhost" << " KICK #" << channel_name << " " << user_name;
		if (!reason.empty())
			ss << " :" << reason << "\r\n";
		else
			ss << "\r\n";

		ch->sendToAll(ss.str());
		if (ch->get_admin(ch->findClientInChannel(user_name)->getFd()))
			ch->removeAdmin(ch->findClientInChannel(user_name)->getFd());
		else
			ch->removeClient(ch->findClientInChannel(user_name)->getFd());

		// Removing a channel from the channels list if it is empty
		for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			if (it->getName() == channel_name && ch->GetClientsNumber() == 0)
			{
				channels.erase(it);
				break;
			}
		}
	}
	else
	{
		sendErrorToClient(441, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :They aren't on that channel\r\n");
	}
}

void Server::kickMultipleUsersFromChannel(const std::string& channel_name, const std::vector<std::string>& users_list, const std::string& reason, const int& _fd)
{
	Channel *ch = getChannel(channel_name);
	if (!ch)
	{
		sendErrorToClient(403, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :No such channel\r\n");
		return;
	}

	if (!ch->get_client(_fd) && !ch->get_admin(_fd))
	{
		sendErrorToClient(442, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :You're not on that channel\r\n");
		return;
	}

	if (!ch->get_admin(_fd))
	{
		sendErrorToClient(482, getClient(_fd)->getNickName(), "#" + channel_name, getClient(_fd)->getFd(), " :You're not channel operator\r\n");
		return;
	}

	for (size_t j = 0; j < users_list.size(); j++)
	{
		kickUserFromChannel(channel_name, users_list[j], reason, _fd);
	}
}

void Server::kickUserFromMultipleChannels(const std::string& user_name, const std::vector<std::string>& channels_list, const std::string& reason, const int& _fd)
{
	for (size_t i = 0; i < channels_list.size(); i++)
	{
		kickUserFromChannel(channels_list[i], user_name, reason, _fd);
	}
}


void Server::cmdKick(const std::vector<std::string>& _params, const std::string& _message, const int &_fd)
{
	std::string reason;
	std::vector<std::string> channels_list;
	std::vector<std::string> users_list;

	if (_params.size() < 2 || _params[0].empty() || _params[1].empty())
		return sendResponse(ERR_NOTENOUGHPARAM(getClient(_fd)->getCommand()), _fd);

	reason = _message;
	channels_list = splitByComma(_params[0]);
	users_list = splitByComma(_params[1]);


	// Check if the number of channels and users is the same
	if (channels_list.size() != 1 && users_list.size() != 1 && channels_list.size() != users_list.size())
	{
		sendErrorToClient(461, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :Number of channels and users must match\r\n");
		return;
	}


	if (channels_list.size() == 1)
	{
		// Single channel, multiple users
		kickMultipleUsersFromChannel(channels_list[0], users_list, reason, _fd);
	}
	else if (users_list.size() == 1)
	{
		// Single user, multiple channels
		kickUserFromMultipleChannels(users_list[0], channels_list, reason, _fd);
	}
	else
	{
		// Multiple users, multiple channels (must match one-to-one)
		for (size_t i = 0; i < channels_list.size(); i++)
		{
			kickUserFromChannel(channels_list[i], users_list[i], reason, _fd);
		}
	}
}
