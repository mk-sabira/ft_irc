#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# define ERR_NOSUCHNICK  401 // No such nick
# define ERR_NOSUCHCHANNEL  403 // No such channel
# define ERR_KEYSET  467 // Channel key already set
# define ERR_CHANNELISFULL  471 // full channel, Cannot join
# define ERR_UNKNOWNMODE  472 // unknown mode char
# define ERR_INVITEONLYCHAN  473 // not invited, Cannot join
# define ERR_BADCHANNELKEY  475 // wrong key, Cannot join

// --------- topic ---------
# define ERR_NEEDMOREPARAMS  461  // not enough parameters
# define ERR_NOTONCHANNEL  442  // not enough parameters
# define ERR_USERONCHANNEL  443  // user is already on channel
# define RPL_NOTOPIC  331  // No topic is set
# define RPL_TOPIC  332  // set Topic
# define ERR_CHANOPRIVSNEEDED  482  //not channel operator

#include <iostream>
#include <string>
#include <stdbool.h>
#include <map>
#include <set>

#include "Client.hpp"



class Channel
{
    private:

        std::string  _name;  //channel name
        std::string  _topic; //description of channel
        std::string  _key;   //optional password to join the channel
        size_t       userLimit;  //max num of users in the channel
        bool         inviteOnly; //invite onlu mode +i
        bool         _topicRestricted;    //topic can be changed? +t
        std::map<int, Client*> _users;   //map of users in the channel with their socket or id
        std::set<int>   _operators;      // set of users (socket, id) who are operators +o
        std::set<int>   _invited;        //which user have been invited, check access for invite-only channels


    public:
        Channel();
        Channel& operator=(Channel& other);
        Channel(Channel& other);
        ~Channel();

        //-----Getters--------------
        std::set<int> getUserList() const;
        std::string getName() const;
        std::string getKey() const;
        std::string getTopic() const;


        //-----Setters--------------
        void setTopic(const std::string& topic);
        // void addOperator(int clientFd);

        //---------------helper functions---------------

        bool isUser(int clientFd) const;
        bool isOperator(int clientFd) const;
        bool isInviteOnly() const;
        bool isInvited(int clientFd) const;
        bool isFull() const;
        bool hasKey() const;
        void removeOperator(int clientFd);

        //--------------------------COMMANDS-----------------------------------------------

        //--------JOIN---------
        bool canJoin(int clientFd, const std::string& key);
        void addUser(int clientFd) ;
        void addOperator(int clientFd) ;
        // bool isFull() const ;
        // bool isInviteOnly() const ;  
        // bool isInvited(int clientFd) const;

        //---------MODE-----------
        void setMode(char mode, bool enable);
        bool hasMode(char mode) const;
        void setKey(const std::string& key);
        bool checkKey(const std::string& key) const;

        //---------------TOPIC----------
        void setTopic(const std::string& topic);
        void clearTopic();
        bool isTopicRestricted() const;

        //----------INVITE--------------
        void inviteUser(int clientFd);
        // bool isInvited(int clientFd) const;

        //-----------KICK------------------
        bool isOperator(int clientFd) const;
        void kickUser(int targetFd);
        


        //-----------PART------------------
        //-----------NAMES------------------
        //-----------LIST------------------
    };



void sendError(int userFd, int errorCode, const std::string& target, const std::string& message = "");


#endif

