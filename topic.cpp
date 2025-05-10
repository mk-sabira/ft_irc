#include "Channel.hpp"
#include <vector>


/* topic:
    TOPIC <channel> [<topic>]
*/

/*
    - Validate Parameters
    - Check Channel Exists
    - Check User Is in Channel
*/

// void Server::topicCommand(int userFd, const std::vector<std::string>& tokens)
void topicCommand(int userFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2)
    {
        sendError(userFd, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
        return ;
    }
    std::string channelName = tokens[1];
    std::map<std::string, Channel>::iterator it = this->_channels.find(channelName);
    if (it == this->_channels.end())
    {
        sendError(userFd, ERR_NOSUCHCHANNEL,  channelName + " :No such channel");
        return ;
    }
    Channel &channel = it->second;
    if (!channel.isUser(userFd))
    {
        sendError(userFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    if (tokens.size() == 2)
    {
        const std::string &topic = channel.getTopic();
        if (topic.empty())
            sendReply(userFd, RPL_NOTOPIC, channelName + " :No topic is set"); // server
        else
            sendReply(userFd, RPL_TOPIC, channelName + " :" + topic); // server
        return;
    }
    if (channel.isTopicRestricted() && !channel.isOperator(userFd))
    {
        sendError(userFd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"); 
        return;
    }
     std::string newTopic = tokens[2]; // assumes already parsed with ":" removed
    for (size_t i = 3; i < tokens.size(); ++i)
    {
        newTopic += " " + tokens[i];
    }
    if (newTopic.empty())
        channel.clearTopic();
    else
        channel.setTopic(newTopic);
    
    // Broadcast topic change to all users in channel
    // std::map<int, Client *>::const_iterator itUser = channel.getUsers().begin();
    // for (; itUser != channel.getUsers().end(); ++itUser)
    // {
    //     int fd = itUser->first;
    //     std::string message = ":" + this->_clients[userFd]->getPrefix() +
    //                           " TOPIC " + channelName + " :" + newTopic;
    //     sendMessage(fd, message);
    }

}