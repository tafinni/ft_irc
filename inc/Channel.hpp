#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <sys/socket.h>
# include <sstream>
# include <vector>
# include <ctime>
# include <iostream>

#include "Client.hpp"
#include "Server.hpp"

class Client;

class Channel
{
private:
    std::string _name;
    std::string _topic_name;
    std::string _password;
   	int _limit;
    std::string _time_creation;
    int _invit_only;
    std::string created_at;
  	std::vector<Client> _clients;
  	std::vector<Client> _admins;
    bool _topicRestriction;

    std::vector<std::pair<char, bool> > _modes;

public:
    Channel();
  	Channel(Channel const &src);
	  Channel &operator=(Channel const &src);
    ~Channel();

    //get methods
    std::string getName();
    std::string getTopicName();
    std::string getPassword();
    int getLimit();
    std::string getTimeCreation();
    int getInvitOnly();
    std::string getCreatiOnTime();
    bool getModeAtindex(size_t index);

    // set methods
    void setName(std::string name_input);
    void setTopicName(std::string topic_name_input);
    void setPassword(std::string password_input);
    void setLimit(int limit_input);
    void setTimeCreation(std::string time_creation_input);
    void setTopicRestriction(bool value);

    void setCreatedAt();
    void setModeAtindex(size_t index, bool mode);
    void setInvitOnly(int invit_only);
    bool getTopicRestriction() const;


    //methods
    void addClient(Client newClient);
	  void addAdmin(Client newClient);
	  void removeClient(int fd);
	  void removeAdmin(int fd);

    Client *get_client(int fd);
	  Client *get_admin(int fd);
    int GetClientsNumber();
    Client*findClientInChannel(const std::string& name);
    bool isClientInChannel(std::string &nick);
    std::string getModes();
    std::string clientChannel_list();

    bool changeClientToAdmin(std::string& nick);
    bool changeAdminToClient(std::string& nick);

    void sendToAll(std::string rpl1);
    void sendToAll(std::string rpl1, int fd);


};




#endif