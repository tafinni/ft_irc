#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <vector>
# include <poll.h>
# include <arpa/inet.h>
# include <iomanip>
# include <unistd.h>
# include <cstring>


# include "Client.hpp"
# include "Channel.hpp"
# include "Replies.hpp"



#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"


class Channel;
class Client;

class Server
{
	private:

		struct sockaddr_in add;
		struct sockaddr_in cliadd;
		struct pollfd new_cli;
		struct pollfd _newpollfd;

		int _port;
		int _socketfd;
		int _clientfd;
		const std::string _password;

		static std::vector<Client> _clients;
		static std::vector<pollfd> _pollfds;
		static std::vector<Channel> channels;

		///

		void setServerSocket();
		void newConnection();
		void receiveMessage(const int fd);
		void executeCommand(std::string &cmd, int fd);

		bool isRegistered(Client *client);
		bool changeNick(std::string& oldnick, std::string& newnick);
		std::vector<std::string> saveMsg(std::string str);

		/* Commands */
		void cmdQuit(Client *client);
		void cmdPart(Client *client);
		void cmdPass(Client *client);
		void cmdUser(Client *client);
		void cmdNick(Client *client);
		void cmdInvite(std::vector<std::string> &_params, const int &_fd);
		void cmdJoin(std::vector<std::string> _params, int _fd);
		void cmdMode(std::vector<std::string> &_params, const std::string& _message, const int &_fd);
		void cmdTopic(const std::vector<std::string>& _params, const std::string& _message, const int &_fd);
		void cmdKick(const std::vector<std::string>& _params, const std::string& _message, const int &_fd);
		void cmdPrivmsg(std::vector<std::string> &_params, const std::string& _message, const int &_fd);

		/* Command utils */
		// JOIN
		int splitJoin(std::vector<std::pair<std::string, std::string> > &token, std::vector<std::string> _params, int _fd);
		int searchForClients(std::string nickname);
		void existChannel(std::vector<std::pair<std::string, std::string> >&token, size_t i, size_t j, int _fd);
		void notExistChannel(std::vector<std::pair<std::string, std::string> >&token, size_t i, int _fd);
		// KICK
		void kickUserFromChannel(const std::string& channel_name, const std::string& user_name, const std::string& reason, const int& _fd);
		void kickMultipleUsersFromChannel(const std::string& channel_name, const std::vector<std::string>& users_list, const std::string& reason, const int& _fd);
		void kickUserFromMultipleChannels(const std::string& user_name, const std::vector<std::string>& channels_list, const std::string& reason, const int& _fd);
		std::vector<std::string> splitByComma(const std::string& str);
		// MODE
		std::vector<std::string> splitParams(std::string message);
		std::string modeToAppend(std::string chain, char opera, char mode);
		std::string inviteOnly(Channel *channel, char opera, std::string chain);
		std::string topicRestriction(Channel *channel ,char opera, std::string chain);
		bool isValidLimit(std::string& limit);
		std::string channelLimit(std::string limit,  Channel *channel, char opera, int fd, std::string chain, std::string& arguments);
		std::string passwordMode(std::string pass, Channel *channel, char opera, int fd, std::string chain, std::string &arguments);
		std::string operatorPrivilege(std::string user, Channel *channel, int fd, char opera, std::string chain, std::string& arguments);
		// PRIVMSG
		void checkChannelsClientsExist(std::vector<std::string> &tmp, int fd);
		// TOPIC
		std::string tTopic();

		/* Get */		
		Channel *getChannel(std::string name);
		Client *getClient(int fd);
		Client *getClientByNickname(std::string nickname);
		std::vector<Client> getClients();

		/* Close and remove */
		void rmChannels(int fd);
		void removeClient(int fd);
		void removeFds(int fd);
		void close_fds();
		void closeClient(Client *client);

	public:
		Server();
		Server(int port, std::string pwd);
		Server(Server const &src);
		Server &operator=(Server const &src);
		~Server();

		void start();

		/* Responses */
		static void sendErrorToClient(int code, std::string clientname, int fd, std::string msg);
		static void sendErrorToClient(int code, std::string nick_name, std::string channelname, int fd, std::string msg);
		static void sendResponse(std::string response, int fd);

};


#endif