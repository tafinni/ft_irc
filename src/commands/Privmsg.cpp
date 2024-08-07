#include "../../inc/Server.hpp"

/*
PRIVMSG is used to send private messages between users, as well as to send messages to channels.

	ERR_NORECIPIENT (411)  if the client doesn't specify the recipient
	ERR_NOTEXTTOSEND (412)  if the client doesn't specify the message
	ERR_TOOMANYTARGETS (407)  if the client send the message to more than 10 clients
	ERR_NOSUCHNICK (401)  if the channel/client doesn't exist
	ERR_CANNOTSENDTOCHAN (404)  if the client is not in the channel
*/

std::vector<std::string> splitByCommaNew(const std::string& str)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, ','))
	{
		if (!item.empty())
		{
			result.push_back(item);
		}
	}
	return result;
}


void	Server::checkChannelsClientsExist(std::vector<std::string> &tmp, int fd)
{
	for(size_t i = 0; i < tmp.size(); i++){
		if (tmp[i][0] == '#'){
			tmp[i].erase(tmp[i].begin());
			if(!getChannel(tmp[i]))
				{sendErrorToClient(401, "#" + tmp[i], getClient(fd)->getFd(), " :No such nick/channel\r\n"); tmp.erase(tmp.begin() + i); i--;}
			else if (!getChannel(tmp[i])->findClientInChannel(getClient(fd)->getNickName()))
				{sendErrorToClient(404, getClient(fd)->getNickName(), "#" + tmp[i], getClient(fd)->getFd(), " :Cannot send to channel\r\n"); tmp.erase(tmp.begin() + i); i--;}
			else tmp[i] = "#" + tmp[i];
		}
		else{
			if (!getClientByNickname(tmp[i]))
				{sendErrorToClient(401, tmp[i], getClient(fd)->getFd(), " :No such nick/channel\r\n"); tmp.erase(tmp.begin() + i); i--;}
		}
	}
}


void Server::cmdPrivmsg(std::vector<std::string> &_params, const std::string& _message, const int &_fd)
{
	std::vector<std::string> tmp;
	tmp = splitByCommaNew(_params[0]);
	if (!tmp.size())
		{sendErrorToClient(411, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :No recipient given (PRIVMSG)\r\n"); return;}
	if (_message.empty())
		{sendErrorToClient(412, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :No text to send\r\n"); return;}
	if (tmp.size() > 10) 
		{sendErrorToClient(407, getClient(_fd)->getNickName(), getClient(_fd)->getFd(), " :Too many recipients\r\n"); return;}
	// check if the channels and clients exist
	checkChannelsClientsExist(tmp, _fd);
	// send the message to the clients and channels
	for (size_t i = 0; i < tmp.size(); i++){
		if (tmp[i][0] == '#'){
			tmp[i].erase(tmp[i].begin());
			std::string resp = ":" + getClient(_fd)->getNickName() + "!~" + getClient(_fd)->getUserName() + "@localhost PRIVMSG #" + tmp[i] + " :" + _message + "\r\n";
			getChannel(tmp[i])->sendToAll(resp, _fd);
		}
		else{
			std::string resp = ":" + getClient(_fd)->getNickName() + "!~" + getClient(_fd)->getUserName() + "@localhost PRIVMSG " + tmp[i] + " :" + _message + "\r\n";
			sendResponse(resp, getClientByNickname(tmp[i])->getFd());
		}
	}
}
