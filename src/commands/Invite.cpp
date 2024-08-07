#include "../../inc/Server.hpp"

/*
Command: INVITE
Parameters: <nickname> <channel>

The INVITE message is used to invite users to a channel.


RPL_INVITING (341)              Returned by the server to indicate that the
								attempted INVITE message was successful and is
								being passed onto the end client.
ERR_NEEDMOREPARAMS (461)        Returned by the server by numerous commands to
								indicate to the client that it didn't supply enough
								parameters.
ERR_NOSUCHCHANNEL (403)         Used to indicate the given channel name is invalid.
ERR_NOTONCHANNEL (442)          Returned by the server whenever a client tries to
								perform a channel effecting command for which the
								client isn't a member.
ERR_CHANOPRIVSNEEDED (482)      Any command requiring 'chanop' privileges (such as
								MODE messages) must return this error if the client
								making the attempt is not a chanop on the specified
								channel.
ERR_USERONCHANNEL (443)         Returned when a client tries to invite a user to a
								channel they are already on.
*/

void Server::cmdInvite(std::vector<std::string> &_params, const int &_fd)
{
	if (_params.size() < 2)
	{
		sendErrorToClient(461, getClient(_fd)->getNickName(), _fd, " :Not enough parameters\r\n");
		return;
	}
	
	std::string channelName = _params[1].substr(1);
	Channel* ch = getChannel(channelName);


	if (_params[1][0] != '#' || !ch)
	{
		sendErrorToClient(403, channelName, _fd, " :No such channel\r\n");
		return;
	}


	if (!(ch->get_client(_fd)) && !(ch->get_admin(_fd))) {
		sendErrorToClient(442, channelName, _fd, " :You're not on that channel\r\n");
		return;
	}
	
	std::string invitedNickname = _params[0];

	if (ch->findClientInChannel(invitedNickname)) {
		sendErrorToClient(443, getClient(_fd)->getNickName(), channelName, _fd, " :is already on channel\r\n");
		return;
	}
	
	Client* invitedClient = getClientByNickname(invitedNickname);
	if (!invitedClient) {
		sendErrorToClient(401, invitedNickname, _fd, " :No such nick\r\n");
		return;
	}

	if (ch->getInvitOnly() && !ch->get_admin(_fd)) 
	{
		sendErrorToClient(482, ch->get_client(_fd)->getNickName(), invitedNickname, _fd, " :You're not channel operator\r\n");
		return;
	}

	if (ch->getLimit() && ch->GetClientsNumber() >= ch->getLimit())
	{
		sendErrorToClient(473, ch->get_client(_fd)->getNickName(), channelName, _fd, " :Cannot invite to channel (+i)\r\n");
		return;
	}
	invitedClient->addChannelInvite(channelName);
	std::string invitingMessage = ": 341 " + getClient(_fd)->getNickName() + " " + invitedNickname + " " + _params[1] + "\r\n";
	sendResponse(invitingMessage, _fd);
	std::string inviteNotification = ":" + invitedClient->getHostName() + " INVITE " + invitedNickname + " " + _params[1] + "\r\n";
	sendResponse(inviteNotification, invitedClient->getFd());
}
