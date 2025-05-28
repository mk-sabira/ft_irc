/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseCmd.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 10:53:08 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/28 10:55:47 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::splitCommand(std::vector<std::string>& tokens, const std::string& command, std::string::size_type start, std::string::size_type end)
{
    while (start < command.length() && (command[start] == ' ' || command[start] == '\t'))
        ++start;
    if(start >= command.length())
        return;
    end = command.find(' ', start);
    if (end == std::string::npos)
    {
        tokens.push_back(command.substr(start));
        return;
    }
    tokens.push_back(command.substr(start, end - start)); // Command name
    start = end + 1;

    while (start < command.length())
    {
        while (start < command.length() && (command[start] == ' ' || command[start] == '\t'))
            ++start;
        
        if (command[start] == ':')
        {
            tokens.push_back(command.substr(start)); // Take rest as single token
            break;
        }
        end = command.find(' ', start);
        if (end == std::string::npos)
        {
            tokens.push_back(command.substr(start));
            break;
        }
        tokens.push_back(command.substr(start, end - start));
        start = end + 1;
    }
}

bool Server::validateNick(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
    {
        return false;
    }
    if (!isalpha(nick[0]) && nick[0] != '[' && nick[0] != ']' && nick[0] != '\\' && 
        nick[0] != '`' && nick[0] != '_' && nick[0] != '^' && nick[0] != '{' && 
        nick[0] != '|' && nick[0] != '}')
    {
        return false;
    }
    for (size_t i = 1; i < nick.length(); ++i)
    {
        if (!isalnum(nick[i]) && nick[i] != '-' && nick[i] != '[' && nick[i] != ']' && 
            nick[i] != '\\' && nick[i] != '`' && nick[i] != '_' && nick[i] != '^' && 
            nick[i] != '{' && nick[i] != '|' && nick[i] != '}')
        {
            // errorMsg = "432 " + nick + " :Erroneous nickname";
            return false;
        }
    }
    return true;
}

bool Server::validateUser(int clientFd, const std::vector<std::string>& tokens, std::string& errorMsg)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    if (tokens.size() != 5)
    {
        errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Not correct parameters";
        return false;
    }
    const std::string& user = tokens[1];
    const std::string& hostname = tokens[2];
    const std::string& serverName = tokens[3];
    const std::string& realname = tokens[4];

    if (user.empty() || user.find(' ') != std::string::npos)
    {
        errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Username contains invalid characters";
        return false;
    }
    for (size_t i = 0; i < user.length(); ++i)
    {
        if (!isalnum(user[i]) && user[i] != '_' && user[i] != '-')
        {
            errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Username contains invalid characters";
            return false;
        }
    }
    if (hostname != "0")
    {
        errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Hostname must be '0'";
        return false;
    }
    if (serverName != "*")
    {
        errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Servername must be '*'";
        return false;
    }
    if (realname.empty())
    {
        errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Realname cannot be empty";
        return false;
    }
    if (realname.find(' ') != std::string::npos && realname[0] != ':')
    {
        errorMsg = macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " USER :Realname with spaces requires ':' prefix";
        return false;
    }
    return true;
}

bool Server::validatePrivmsg(int senderFd, const std::vector<std::string>& tokens, std::string& errorMsg)
{
    std::string nick = _clients[senderFd]->getNickname().empty() ? "*" : _clients[senderFd]->getNickname();
    if (tokens.size() < 2 )
    {
        errorMsg = macroToString(ERR_NOTEXTTOSEND) + " " + nick + " PRIVMSG :No recipient given";
        return false;
    }
    if (tokens.size() < 3 || tokens[2].empty())
    {
        errorMsg = macroToString(ERR_NOTEXTTOSEND) + " " + nick + " PRIVMSG :No text to send";
        return false;
    }
    return true;
}


std::string Server::buildPrivmsg(const std::vector<std::string>& tokens)
{
    std::string message;
    if (tokens[2][0] == ':')
        message = tokens[2].substr(1);
    else
        message = tokens[2];
    for (size_t i = 3; i < tokens.size(); i++)
        message += " " + tokens[i];
    return (message);
}

void Server::sendToChannelTarget(int senderFd, const std::string& target, const std::string& message)
{
    std::string nick = _clients[senderFd]->getNickname().empty() ? "*" : _clients[senderFd]->getNickname();
    std::map<std::string, Channel*>::iterator it = _channels.find(target);
    if (it == _channels.end())
    {
        sendReply(senderFd, macroToString(ERR_NOSUCHCHANNEL) + " " + nick + " " + target + " :No such channel");
        return;
    }
    
    Channel* channel = it->second;
    if (!channel->isUser(senderFd))
    {
        sendReply(senderFd, macroToString(ERR_NOTONCHANNEL) + " " + nick + " " + target + " :You're not on that channel");
        return;
    }
    std::string msg = ":" + _clients[senderFd]->getPrefix() + " PRIVMSG " + target + " :" + message;
    channel->boolBroadCastToAll(msg, this, false);
}

void Server::sendToClientTarget(int senderFd, const std::string& target, const std::string& message)
{
    std::string nick = _clients[senderFd]->getNickname().empty() ? "*" : _clients[senderFd]->getNickname();
    Client* targetClient = getClientByNickname(target);
    if (!targetClient)
    {
        std::string reply = macroToString(ERR_NOSUCHNICK) + " " + nick + " " + target + " :No such nick";
        sendReply(senderFd, reply);
        return;
    }
    std::string msg = ":" + _clients[senderFd]->getPrefix() + " PRIVMSG " + target + " :" + message;
    boolSendReply(targetClient->getFd(), msg, false);
}

void Server::parseJoinCommand(int userFd, const std::string& command)
{
    std::vector<std::string> tokens;
    size_t                   start = 0;

    while (start < command.length() && command[start] == ' ')
        ++start;
    size_t end = command.find(' ', start);
    if (end == std::string::npos)
    {
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
        return;
    }

    start = end + 1;
    while (start < command.length() && command[start] == ' ')
        ++start;
    if (start >= command.length()) {
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
        return;
    }
    //extract channel list
    end = command.find(' ', start);
    std::string channels = command.substr(start, end - start);
    //extract keys
    std::string keys;
    if (end != std::string::npos)
    {
        start = end + 1;
        while (start < command.length() && command[start] == ' ')
            ++start;
        if (start < command.length())
            keys = command.substr(start);
    }

    // Split channels
    std::vector<std::string> channelList;
    std::string::size_type chanStart = 0;
    std::string::size_type chanEnd;
    while ((chanEnd = channels.find(',', chanStart)) != std::string::npos)
    {
        channelList.push_back(channels.substr(chanStart, chanEnd - chanStart));
        chanStart = chanEnd + 1;
    }
    if (chanStart < channels.length())
        channelList.push_back(channels.substr(chanStart));

    // Split keys
    std::vector<std::string> keyList;
    std::string::size_type keyStart = 0;
    std::string::size_type keyEnd;
    while ((keyEnd = keys.find(',', keyStart)) != std::string::npos)
    {
        keyList.push_back(keys.substr(keyStart, keyEnd - keyStart));
        keyStart = keyEnd + 1;
    }
    if (keyStart < keys.length())
        keyList.push_back(keys.substr(keyStart));

    // Join each channel with corresponding key
    for (std::size_t i = 0; i < channelList.size(); ++i)
    {
        std::string key = (i < keyList.size()) ? keyList[i] : "";
        joinCommand(userFd, channelList[i], key);
    }
}

void Server::parseTopicCommand( int userFd, const std::string& command)
{
   std::vector<std::string> tokens;
    size_t                   start = 0;

    while (start < command.length() && command[start] == ' ')
        ++start;
    size_t end = command.find(' ', start);
    if (end == std::string::npos)
    {
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
        return;
    }

    start = end + 1;
    while (start < command.length() && command[start] == ' ')
        ++start;

    if (start >= command.length()) // Missing channel parameter
    {
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "TOPIC: Not enough parameters");
        return;
    }

    // Extract channel
    end = command.find(' ', start);
    std::string channel = command.substr(start, end - start);
    start = (end == std::string::npos) ? std::string::npos : end + 1;

    // Skip spaces before topic
    while (start != std::string::npos && start < command.length() && command[start] == ' ')
        ++start;

    // Extract topic if provided (including colon)
    std::string topic = "";
    bool    colon = false;
    if (start != std::string::npos && start < command.length())
    {
        if (command[start] == ':')
        {
            colon = true;
            ++start; // Skip colon
        }
        topic = command.substr(start);
    }

    // Pass the extracted channel and topic (possibly empty) to the handler
    topicCommand(userFd, channel, topic, colon);
}