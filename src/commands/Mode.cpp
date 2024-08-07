#include "../../inc/Server.hpp"

/*
	 Parameters: <channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
			   [<ban mask>]

   The MODE command is provided so that channel operators may change the
   characteristics of `their' channel.  It is also required that servers
   be able to change channel modes so that channel operators may be
   created.

   The various modes available for channels are as follows:

		i - invite-only channel flag;
		t - topic settable by channel operator only flag;
		k - set a channel key (password).
		o - give/take channel operator privileges;
		l - set the user limit to channel;
*/

std::vector<std::string> Server::splitParams(std::string message)
{
	std::vector<std::string> tokens;
	std::string param;

	std::istringstream stm(message);
	while (std::getline(stm, param, ',') || std::getline(stm, param, ' '))
	{
		tokens.push_back(param);
		param.clear();
	}
	return tokens;
}


std::string Server::modeToAppend(std::string chain, char opera, char mode)
{
	std::stringstream ss;

	ss.clear();
	char last = '\0';
	for(size_t i = 0; i < chain.size(); i++)
	{
		if(chain[i] == '+' || chain[i] == '-')
			last = chain[i];
	}
	if(last != opera)
		ss << opera << mode;
	else
		ss << mode;
	return ss.str();
}



std::string Server::inviteOnly(Channel *channel, char opera, std::string chain)
{
	std::string result;
	result.clear();

	if (opera == '+')
	{
		if (!channel->getModeAtindex(0))
		{
			channel->setModeAtindex(0, true);
			channel->setInvitOnly(1);
			result = modeToAppend(chain, opera, 'i');
		}
	}
	else if (opera == '-') 
	{
		if (channel->getModeAtindex(0)) 
		{
			channel->setModeAtindex(0, false);
			channel->setInvitOnly(0);
			result = modeToAppend(chain, opera, 'i');
		}
	}
	return result;
}


std::string Server::topicRestriction(Channel *channel ,char opera, std::string chain)
{
	std::string param;
	param.clear();
	if(opera == '+' && !channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, true);
		channel->setTopicRestriction(true);
		param =  modeToAppend(chain, opera, 't');
	}
	else if (opera == '-' && channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, false);
		channel->setTopicRestriction(false);
		param =  modeToAppend(chain, opera, 't');
	}	
	return param;
}

bool Server::isValidLimit(std::string& limit)
{
	return (!(limit.find_first_not_of("0123456789")!= std::string::npos) && std::atoi(limit.c_str()) > 0);
}


std::string Server::channelLimit(std::string limit,  Channel *channel, char opera, int fd, std::string chain, std::string& arguments)
{
	std::string param;

	if(opera == '+')
	{
		if(!limit.empty())
		{
			if(!isValidLimit(limit))
			{
				sendResponse(ERR_INVALIDMODEPARM(channel->getName(),"(l)"), fd);
			}
			else
			{
				channel->setModeAtindex(4, true);
				channel->setLimit(std::atoi(limit.c_str()));
				if(!arguments.empty())
					arguments += " ";
				arguments += limit;
				param =  modeToAppend(chain, opera, 'l');
			}
		}
		else
			sendResponse(ERR_NEEDMODEPARM(channel->getName(),"(l)"),fd);
	}
	else if (opera == '-' && channel->getModeAtindex(4))
	{
		channel->setModeAtindex(4, false);
		channel->setLimit(0);
		param  = modeToAppend(chain, opera, 'l');
	}
	return param;

	param.clear();
}

bool validPassword(std::string password)
{
	if(password.empty())
		return false;
	for(size_t i = 0; i < password.size(); i++)
	{
		if(!std::isalnum(password[i]) && password[i] != '_')
			return false;
	}
	return true;
}


std::string Server::passwordMode(std::string pass, Channel *channel, char opera, int fd, std::string chain, std::string &arguments)
{
	std::string param;


	param.clear();
	if(pass.empty())
	{
		sendResponse(ERR_NEEDMODEPARM(channel->getName(),std::string("(k)")),fd);
		return param;
	}

	if(!validPassword(pass))
	{
		sendResponse(ERR_INVALIDMODEPARM(channel->getName(),std::string("(k)")),fd);
		return param;
	}
	if(opera == '+')
	{
		channel->setModeAtindex(2, true);
		channel->setPassword(pass);

		if(!arguments.empty())
			arguments += " ";
		arguments += pass;
		param = modeToAppend(chain, opera, 'k');
	}
	else if (opera == '-' && channel->getModeAtindex(2))
	{
		if(pass == channel->getPassword())
		{		
			channel->setModeAtindex(2, false);
			channel->setPassword("");

			param = modeToAppend(chain, opera, 'k');
		}
		else
			sendResponse(ERR_KEYSET(channel->getName()), fd);
	}
	return param;
}


std::string Server::operatorPrivilege(std::string user, Channel *channel, int fd, char opera, std::string chain, std::string& arguments)
{

	std::string param;

	param.clear();
	if(user.empty())
	{
		sendResponse(ERR_NEEDMODEPARM(channel->getName(),"(o)"),fd);
		return param;
	}
	if(!channel->isClientInChannel(user))
	{
		sendResponse(ERR_NOSUCHNICK(channel->getName(), user),fd);
		return param;
	}
	if(opera == '+')
	{

		channel->setModeAtindex(3,true);
		if(channel->changeClientToAdmin(user))
		{
			param = modeToAppend(chain, opera, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	else if (opera == '-')
	{
		channel->setModeAtindex(3,false);
		if(channel->changeAdminToClient(user))
		{
			param = modeToAppend(chain, opera, 'o');
				if(!arguments.empty())
					arguments += " ";
			arguments += user;
		}
	}
	return param;
}



void Server::cmdMode(std::vector<std::string> &_params, const std::string& _message, const int &_fd)
{
	std::string channelName;
	std::string modeset;
	std::string paramsStr;
	std::stringstream mode_chain;
	std::string arguments = ":";
	Channel* channel;
	char opera = '\0';
	std::string limit;
	std::string pass;
	std::string user;

	Client *cli = getClient(_fd);

	if(_params.size() < 2)
	{
		sendResponse(ERR_NOTENOUGHPARAM(cli->getNickName()), _fd); 
		return;
	}

	arguments.clear();
	mode_chain.clear();

	
	channelName = _params[0];
	modeset = _params[1];

	std::vector<std::string> tokens = splitParams(_message);
	if(channelName[0] != '#' || !(channel = getChannel(channelName.substr(1))))
	{
		sendResponse(ERR_CHANNELNOTFOUND(cli->getUserName(), channelName), _fd);
		return ;
	}
	else if (!channel->get_client(_fd) && !channel->get_admin(_fd))
	{
		sendErrorToClient(442, getClient(_fd)->getNickName(), channelName, getClient(_fd)->getFd(), " :You're not on that channel\r\n"); return;
	}
	// response with the channel modes (MODE #channel)
	else if (modeset.empty()) 
	{
		// Send a reply with channel modes.
		sendResponse(RPL_CHANNELMODES(cli->getNickName(), channel->getName(), channel->getModes()) + \
		RPL_CREATIONTIME(cli->getNickName(), channel->getName(),channel->getCreatiOnTime()),_fd);
		return ;
	}
	// If the client does not have channel operator privileges
	else if (!channel->get_admin(_fd))
	{
		sendResponse(ERR_NOTOPERATOR(channel->getName()), _fd);
		return ;
	}
	else if (channel)
	{
		
        size_t pos = 0;
		for (size_t i = 0; i < modeset.size(); i++)
		{
			if (modeset[i] == '+' || modeset[i] == '-')
			{
				opera = modeset[i];
			}
			else
			{
				if (modeset[i] == 'i')
				{
					mode_chain << inviteOnly(channel, opera, mode_chain.str());
				}
				else if (modeset[i] == 't')
				{
					mode_chain << topicRestriction(channel, opera, mode_chain.str());
				}
				else if (modeset[i] == 'k')
				{
					if (_params.size() > pos+2)
						pass = _params[pos+2];
					else if (tokens.size() > pos)
						pass = tokens[pos];
					else
						pass = "";
					mode_chain <<  passwordMode(pass, channel, opera, _fd, mode_chain.str(), arguments);
					pos++;
				}
				else if (modeset[i] == 'o')
				{
					if (_params.size() > pos+2)
						user = _params[pos+2];
					else if (tokens.size() > pos)
						user = tokens[pos];
					else
						user = "";
					mode_chain << operatorPrivilege(user, channel, _fd, opera, mode_chain.str(), arguments);
					pos++;
				}
				else if (modeset[i] == 'l')
				{
					if (_params.size() > pos+2)
						limit = _params[pos+2];
					else if (tokens.size() > pos)
						limit = tokens[pos];
					else
						limit = "";
					mode_chain << channelLimit(limit, channel, opera, _fd, mode_chain.str(), arguments);
					pos++;
				}
				else
				{
					sendResponse(ERR_UNKNOWNMODE(cli->getNickName(), channel->getName(),modeset[i]),_fd);
				}
			}
		}
	}

	std::string chain = mode_chain.str();
	if(chain.empty())
		return ;
 	channel->sendToAll(RPL_CHANGEMODE(cli->getHostName(), channel->getName(), mode_chain.str(), arguments));

}
