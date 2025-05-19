#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

// ------------ helper functions ----------

std::string vecToStr(std::vector<std::string> vec) // Taha changed form string& to string
{
    std::string str;

    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin())
            str += " ";
        str += *it;
    }
    return (str);
}

// void Server::sendError(int userFd, int errorCode, const std::string& message) // Taha Error compilation
// {
//     std::string nickname = getClientNickname(userFd); // safely fallback if not registered
//     std::stringstream ss;

//     ss << ":" << _serverName << " " << errorCode << " "
//        << (nickname.empty() ? "*" : nickname) << " "
//        << message;

//     sendReply(userFd, ss.str());
// }


// void Server::sendToClient(int fd, int code, const std::string& message) { // Taha compile error!
//     std::stringstream ss;
//     std::string nickname =  getClientNickname(fd); 

//     ss << ":" << _serverName << " "
//        << code << " "
//        << nickname << " "
//        << message << "\r\n";

//     sendReply(fd, ss.str());
// }


void Server::broadcastToAll(const Channel& channel, const std::string& msg, int excludeFd) // Taha compilation Error
{
    const std::set<int>& users = channel.getUserFds();

    for (std::set<int>::const_iterator it = users.begin(); it != users.end(); ++it) {
        if (*it != excludeFd)
            sendMessage(*it, msg);
    }
}

// std::string Server::getClientPrefix(int fd) const // comented by Taha compile Error!
// {
//     std::map<int, Client>::const_iterator it = _clients.find(fd);
//     if (it != _clients.end() && it->second != NULL)
//         return it->second.getPrefix();
//     return "";
// }

std::string Server::getClientPrefix(int fd) const // Taha fixed
{
    std::map<int, Client>::const_iterator it = _clients.find(fd);
    if (it != _clients.end())
        return it->second.getPrefix();
    return "";
}


//---------------------------------- COMMANDS ----------------------------------------------------------------------------------------


void Server::joinCommand(int userFd, std::string channelName, std::string key) // Taha compilation error
{
    if (channelName.empty() || channelName[0] != '#')
    {
        sendError(userFd, ERR_NOSUCHCHANNEL, channelName);
        return;
    }

    Channel* channel = nullptr;
    bool isNewChannel = false;

    if (_channels.find(channelName) == _channels.end())
    {
        channel = new Channel(channelName);

        // Optional: set key if provided
        // if (!key.empty())
        //     channel->setKey(key);

        _channels.insert(std::pair<std::string, Channel*>(channelName, channel));
        isNewChannel = true;

        channel->addUser(userFd, &_clients[userFd]);
        channel->addOperator(userFd);
    }
    else
    {
        channel = _channels[channelName];

        if (channel->isUser(userFd))
            return;

        if (channel->isInviteOnly() && !channel->isInvited(userFd)) {
            sendError(userFd, ERR_INVITEONLYCHAN, channelName);
            return;
        }
        if (!channel->canJoin(userFd, key)){
            sendError(userFd, ERR_BADCHANNELKEY, channelName);
            return;
        }
        if (channel->isFull()) {
            sendError(userFd, ERR_CHANNELISFULL, channelName);
            return;
        }
        channel->addUser(userFd, &_clients[userFd]);
    }
    std::string prefix = channel->getClientPrefix(userFd);
    std::string joinMsg = ":" + prefix + " JOIN " + channelName;
    channel->broadcastToAll(joinMsg, this);

    if (!(channel->getTopic().empty()))
        sendToClient(userFd, RPL_TOPIC , " " + channelName + " :" + channel->getTopic());
    else
        sendToClient(userFd, RPL_NOTOPIC , " " + channelName);

    std::string userList = vecToStr(channel->getNicknamesWithPrefixes());
    sendToClient(userFd, RPL_NAMREPLY , " = " + channelName + " :" + userList); // send to client
    sendToClient(userFd, RPL_ENDOFNAMES , " " + channelName + " :End of /NAMES list."); // send to client
}

//-------------------------------------------------------------------------------------------------------------------------------------

/* topic:
        TOPIC <channel> [<topic>]
*/


// void Server::topicCommand(int userFd, const std::vector<std::string>& tokens) commented by Taha compile error!
// {
//     if (tokens.size() < 2)
//     {
//         sendError(userFd, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
//         return ;
//     }
//     std::string channelName = tokens[1];
//     std::map<std::string, Channel*>::iterator it = this->_channels.find(channelName);
//     if (it == this->_channels.end())
//     {
//         sendError(userFd, ERR_NOSUCHCHANNEL,  channelName + " :No such channel");
//         return ;
//     }
//     Channel &channel = *(it->second);
//     if (!channel.isUser(userFd))
//     {
//         sendError(userFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
//         return;
//     }
//     if (tokens.size() == 2)
//     {
//         const std::string &topic = channel.getTopic();
//         if (topic.empty())
//             sendToClient(userFd, RPL_NOTOPIC, " " + channel.getName() + " :No topic is set"); // server
//         else
//             sendToClient(userFd, RPL_TOPIC, " " + channelName + " :" + topic); // server
//         return;
//     }
//     if (channel.isTopicRestricted() && !channel.isOperator(userFd))
//     {
//         sendError(userFd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"); 
//         return;
//     }
//      std::string newTopic = tokens[2]; // assumes already parsed with ":" removed
//     for (size_t i = 3; i < tokens.size(); ++i)
//     {
//         newTopic += " " + tokens[i];
//     }
//     if (newTopic.empty())
//         channel.clearTopic();
//     else
//         channel.setTopic(newTopic);
    
//     std::string msg = ":" + _clients[userFd]->getPrefix() + " TOPIC " + channelName + " :" + newTopic;
//     broadcastToAll(channel, msg, -1);

// }

//-------------------------------------------------------------------------------------------------------------------------------------
/* 
    syntax: 
        INVITE <nickname> <channel>
*/

// void Server::inviteCommand(int senderFd, const std::vector<std::string>& tokens) // Taha compilation Error
// {
//     if (tokens.size() < 3)
//     {
//         sendError(senderFd, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
//         return;
//     }
    
//     std::string targetNick = tokens[1];
//     std::string channelName = tokens[2];
    
//     Client* targetClient = getClientByNickname(targetNick);
//     if (!targetClient)
//     {
//         sendError(senderFd, ERR_NOSUCHNICK, targetNick + " :No such nick");
//         return;
//     }

//     std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
//     if (it == _channels.end())
//     {
//         sendError(senderFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
//         return;
//     }

//     Channel& channel = *(it->second);

//     if (!channel.isUser(senderFd))
//     {
//         sendError(senderFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel"); // ERR_NOTONCHANNEL
//         return;
//     }

//     if (channel.isInviteOnly() && !channel.isOperator(senderFd))
//     {
//         sendError(senderFd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
//         return;
//     }

//     int targetFd = targetClient->getFd();

//     if (channel.isUser(targetFd))
//     {
//         sendError(senderFd, ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
//         return;
//     }
//     channel.addInvite(targetFd);

//     // Notify inviter
//     sendToClient(senderFd, RPL_INVITING, targetNick + " " + channelName); //invite reply

//     // Notify invitee // commented by Taha compile error
//     // std::string inviteMsg = ":" + _clients[senderFd]->getPrefix() + 
//     // " INVITE " + targetNick + " :" + channelName;
//     // sendMessage(targetFd, inviteMsg);
// }

/* syntax:
    KICK <channel>{,<channel>} <user>{,<user>} [<comment>] // commented by dina
*/

void Server::kickCommand(int senderFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 3)
    {
        sendReply(senderFd, "461 KICK :Not enough parameters");
        return;
    }

    std::string channelName = tokens[1];
    std::string targetNick = tokens[2];
    std::string reason = (tokens.size() >= 4) ? tokens[3].substr(1) : _clients[senderFd].getNickname();

    std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
    {
        sendReply(senderFd, "403 " + channelName + " :No such channel");
        return;
    }

    Channel* channel = it->second;
    if (!channel->isUser(senderFd))
    {
        sendReply(senderFd, "442 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel->isOperator(senderFd))
    {
        sendReply(senderFd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    Client* targetClient = getClientByNickname(targetNick);
    if (!targetClient)
    {
        sendReply(senderFd, "401 " + targetNick + " :No such nick");
        return;
    }

    int targetFd = targetClient->getFd();
    if (!channel->isUser(targetFd))
    {
        sendReply(senderFd, "441 " + targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    std::string msg = ":" + _clients[senderFd].getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
    channel->broadcastToAll(msg, this);

    channel->removeUser(targetFd);
}