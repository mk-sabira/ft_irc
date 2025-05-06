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
        std::map<int, Client*> users;   //map of users in the channel with their socket or id
        std::set<int>   operators;      // set of users (socket, id) who are operators +o
        std::set<int>   invited;        //which user have been invited, check access for invite-only channels


    public:
        Channel(/* args */);
        ~Channel();

        // std::set<int> getUserList() const;
        // bool isOperator(int clientFd) const;
        // void addOperator(int clientFd);
        // void removeOperator(int clientFd);
        // std::string getName() const;
};



#endif

