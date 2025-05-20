#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# define RPL_NOTOPIC  331  // No topic is set
# define RPL_TOPIC  332  // set Topic
# define RPL_INVITING  341  //invite reply
# define RPL_NAMREPLY  353  //End of NAMES list"
# define RPL_ENDOFNAMES  366  //End of NAMES list"
# define ERR_NOSUCHNICK  401 // No such nick
# define ERR_NOSUCHCHANNEL  403 // No such channel
# define ERR_USERNOTINCHANNEL  441  //user isn't on that channel
# define ERR_NOTONCHANNEL  442  // not enough parameters
# define ERR_USERONCHANNEL  443  // user is already on channel
# define ERR_KEYSET  467 // Channel key already set
# define ERR_NEEDMOREPARAMS  461  // not enough parameters
# define ERR_ALREADYREGISTRED  462  // :Unauthorized command (already registered)
# define ERR_CHANNELISFULL  471 // full channel, Cannot join
# define ERR_UNKNOWNMODE  472 // unknown mode char
# define ERR_INVITEONLYCHAN  473 // not invited, Cannot join
# define ERR_BADCHANNELKEY  475 // wrong key, Cannot join
# define ERR_CHANOPRIVSNEEDED  482  //not channel operator


#include <iostream>
#include <string>
#include <stdbool.h>
#include <map>
#include <set>
#include <vector>

#include "Client.hpp"

class Server;


class Channel
{
    private:
        std::string  _name;  //channel name
        std::string  _topic; //description of channel
        std::string  _key;   //optional password to join the channel
        int       _userLimit;  //max num of users in the channel
        bool         _inviteOnly; //invite onlu mode +i
        bool         _topicRestricted;    //topic can be changed? +t
        std::map<int, Client*> _users;   //map of users in the channel with their socket or id
        std::set<int>   _operators;      // set of users (socket, id) who are operators +o
        std::set<int>   _invited;        //which user have been invited, check access for invite-only channels
        
        Channel& operator=(Channel& other);
        Channel(Channel& other);
        
    public:
        Channel();
        Channel(const std::string& name);
        ~Channel();
        
        //-----Getters--------------
        std::set<int>	getUserFds() const;
        std::string getName() const;
        std::string getKey() const;
        std::string getTopic() const;
        std::vector<std::string>    getNicknamesWithPrefixes() const;
        std::string getClientPrefix(int fd) const;


        //-----Setters--------------
        void	setName(const std::string& channelName);
        void	setTopic(const std::string& topic);
        void	setKey(const std::string& key);
        void	setInviteFlag(const char   sign);
        void	setRestrictions(const char   sign);
        void	setKeyMode(const char   sign, const std::string& key);
        void	setOperatorMode(const char   sign, int userFd);
        void	setUserLimit(const char   sign, int limit);
		
        //---------------helper functions---------------
		
        bool	isUser(int clientFd) const;
        bool	isOperator(int clientFd) const;
        bool	isInviteOnly() const;
        bool	isInvited(int clientFd) const;
        bool	isTopicRestricted() const;
        bool	isFull() const;
        bool	canJoin(const std::string& key);
        bool	hasKey() const;
        void	removeUser(int clientFd);
        void	removeOperator(int clientFd);
        void	addUser(int clientFd, Client* client);
        void	addOperator(int clientFd) ;
        void	addInvite(int clientFd) ;
        void	clearTopic();
        void	inviteUser(int clientFd);
        void 	kickUser(int targetFd);
        void    broadcastToAll(const std::string& message, Server* server);
};


#endif