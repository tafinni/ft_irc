#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <sys/socket.h>
# include <sstream>
# include <vector>
# include <iostream>
# include <exception>


# include "Server.hpp"


class Client
{
	private:
		// info saved at first connection and registration
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _ip;
		int	_fd;
		bool _registered = false;
		bool _passed;
		bool _nicked;

		// incoming message buffers
		std::string _buffer;
		bool _trailing_flag;

		// parsed message
		std::string _command;
		std::vector<std::string> _params;
		std::string _message;

		std::vector<std::string> _channelsInvite;

	public:
		Client();
		Client(Client const &src);
		Client &operator=(Client const &src);

		~Client();

		void parseMsg(const std::string &cmd);
		void addChannelInvite(std::string &chname);
		void addToBuffer(char c);
		void rmChannelInvite(std::string &chname);
		void clearBuffer();
		void clearMessageData();

		/* Get */
		bool getNicked();
    	bool getTrailingFlag() const;
		bool getInviteChannel(std::string &ChName);
		std::string getCommand() const;
		std::vector<std::string>& getParams();
		std::string& getMessage();
		std::string getNickName();
		std::string& getBuffer();
		const std::string& getIp() const;
		const int& getFd() const;
		const bool& getRegistered() const;
		std::string getHostName();
		std::string getUserName();
		std::string getRealName() const;

		bool getPassed();

		/* Set */
		void setBuffer(std::string buff);
		void setPassed(bool pass);
		void setNicked(bool nick);
  	void setFd(int fd);
    void setIp(std::string IPaddr);
		void setUserName(std::string& username);
		void setNickName(std::string& nickName);
		void setRegistered(bool value);
		void setRealName(std::string& realname);
		void setTrailingFlag(bool flag);

		/* Exceptions */
		class BackToLoop: public std::exception
		{
			public:
				const char *what() const throw();
		};
};

#endif
