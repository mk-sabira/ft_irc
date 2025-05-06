#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <string>
#include <stdbool.h>
#include <map>
#include <set>


class Channel
{
    private:

        std::string  _name;  //channel name
        std::string  _topic; //description of channel
        std::string  _key;   //optional password to join the channel
        size_t       userLimit;  //max num of users in the channel
        bool         inviteOnly; //invite onlu mode +i
        bool         topicRestricted;    //topic can be changed? +t
        std::map<int, Client*> _users;   //map of users in the channel with their socket or id
        std::set<int>   _operators;      // set of users (socket, id) who are operators +o
        std::set<int>   invited;        //which user have been invited, check access for invite-only channels


    public:
        Channel();
        Channel& operator=(Channel& other);
        Channel(Channel& other);
        ~Channel();

        //-----Getters--------------
        std::set<int> getUserList() const;
        std::string getName() const;


        //-----Setters--------------
        bool isOperator(int clientFd) const;
        void addOperator(int clientFd);
        void removeOperator(int clientFd);

        //--------------------------COMMANDS-----------------------------------------------

        //--------JOIN---------
        bool canJoin(int clientFd, const std::string& key);
        void addUser(int clientFd) ;
        bool isFull() const ;
        bool isInviteOnly() const ;  
        bool isInvited(int clientFd) const;

        //---------MODE-----------
        void setMode(char mode, bool enable);
        bool hasMode(char mode) const;
        void setKey(const std::string& key);
        bool checkKey(const std::string& key) const;

        //---------------TOPIC----------
        void setTopic(const std::string& topic);
        const std::string& getTopic() const;
        bool isTopicLocked() const;

        //----------INVITE--------------
        void inviteUser(int clientFd);
        bool isInvited(int clientFd) const;

        //-----------KICK------------------
        bool isOperator(int clientFd) const;
        void kickUser(int targetFd);
        


        //-----------PART------------------
        //-----------NAMES------------------
        //-----------LIST------------------
    };



#endif

