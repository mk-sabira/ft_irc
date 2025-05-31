/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   processCmd.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 10:59:00 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/30 13:26:06 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handlePass(int clientFd, const std::vector<std::string>& tokens)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    if (tokens.size() != 2 || tokens[1].empty())
    {
        sendReply(clientFd, macroToString(ERR_NEEDMOREPARAMS) +  " " + nick + " PASS :Not enough parameters");
        return ;
    }
    if (_clients[clientFd]->isAuthenticated())
    {
        sendReply(clientFd, macroToString(ERR_ALREADYREGISTERED) + " " + nick + " :Already Authenticated");
        return ;
    }
    if (tokens[1] != getPassword())
    {
        sendReply(clientFd, macroToString(ERR_PASSWDMISMATCH) + " " + nick + " :Password incorrect");
        return ;
    }
    _clients[clientFd]->setAuthenticated(true);
}



void Server::handleNick(int clientFd, const std::vector<std::string>& tokens)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();

    if (tokens.size() < 2 || tokens[1].empty())
    {
        sendReply(clientFd, macroToString(ERR_NONICKNAMEGIVEN) + " " + nick + " :No nickname given");
        return;
    }
    std::string newNick = tokens[1];
    if (!validateNick(newNick))
    {
        sendReply(clientFd, macroToString(ERR_ERRONEUSNICKNAME) + " " + newNick + " :Erroneous nickname");
        return ;
    }
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == newNick && it->first != clientFd)
        {
            sendReply(clientFd, macroToString(ERR_NICKNAMEINUSE) + " " + newNick + " :Nickname is already in use");
            return;
        }
    }
    
    Client* cleint = _clients[clientFd];
    std::string oldNick = cleint->getNickname();
    cleint->setNickname(newNick);
    
    if (_clients[clientFd]->isRegistered())
        notifyChangeNick(clientFd, oldNick, newNick);
    
    if (_clients[clientFd]->isAuthenticated() && 
        !_clients[clientFd]->getNickname().empty() && 
        !_clients[clientFd]->getUsername().empty() &&
        !_clients[clientFd]->isRegistered())
    {
        _clients[clientFd]->setRegistered(true);
        sendReply(clientFd, "001 " + _clients[clientFd]->getNickname() + " :Welcome to the IRC server");
        sendReply(clientFd, "002 " + _clients[clientFd]->getNickname() + " :Your host is " + _serverName);
        sendReply(clientFd, "003 " + _clients[clientFd]->getNickname() + " :This server was created today");
        sendReply(clientFd, "004 " + _clients[clientFd]->getNickname() + " :" + _serverName + " 1.0");
        std::cout << CYAN << "New client connected: FD = " << clientFd << RESET << std::endl;
    }
}

void Server::handleUser(int clientFd, const std::vector<std::string>& tokens)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    std::string errorMsg;
    if (_clients[clientFd]->isRegistered())
    {
        sendToClient(clientFd, ERR_ALREADYREGISTRED, " * " + tokens[1] + ":already registered");
        return;
    }
    if (!validateUser(clientFd, tokens, errorMsg))
    {
        sendReply(clientFd, errorMsg);
        return;
    }
    _clients[clientFd]->setUsername(tokens[1]);
    _clients[clientFd]->setRealname(tokens[4]);
    if (_clients[clientFd]->isAuthenticated() && !_clients[clientFd]->getNickname().empty() && !_clients[clientFd]->getUsername().empty() && !_clients[clientFd]->isRegistered())
    {
        _clients[clientFd]->setRegistered(true);
        sendReply(clientFd, "001 " + _clients[clientFd]->getNickname() + " :Welcome to the IRC server");
        sendReply(clientFd, "002 " + _clients[clientFd]->getNickname() + " :Your host is " + _serverName);
        sendReply(clientFd, "003 " + _clients[clientFd]->getNickname() + " :This server was created today");
        sendReply(clientFd, "004 " + _clients[clientFd]->getNickname() + " :" + _serverName + " 1.0");
        std::cout << CYAN << "New client connected: FD = " << clientFd << RESET << std::endl;
    }
}
    
void Server::handlePrivmsg(int senderFd, const std::vector<std::string>& tokens)
{
    if (!_clients[senderFd]->isRegistered())
    {
        sendReply(senderFd, macroToString(ERR_NOTREGISTERED) + " * :You have not registered"); 
        return;
    }
    std::string errorMsg;
    if (!validatePrivmsg(senderFd, tokens, errorMsg))
    {
        sendReply(senderFd, errorMsg);
        return;
    }
    std::string targetNick = tokens[1];
    std::string message = buildPrivmsg(tokens);
    Client* tragetClient = getClientByNickname(targetNick);
    if (tokens[1][0] != '#' && tragetClient && !_clients[tragetClient->getFd()]->isRegistered())
    {
        sendReply(senderFd, macroToString(ERR_NOTREGISTERED) + " * :The user is not registered"); 
        return;
    }
    if(targetNick[0] == '#')
        sendToChannelTarget(senderFd, targetNick, message);
    else
        sendToClientTarget(senderFd, targetNick, message);
        
}

void Server::handlePart(int clientFd, const std::string& command)
{
    if (!_clients[clientFd]->isRegistered())
    {
        sendReply(clientFd, macroToString(ERR_NOTREGISTERED) + " * :You have not registered"); 
        return;
    }
    std::vector<std::string> tokens;
    size_t                   start = 0;
    
    while (start < command.length() && command[start] == ' ')
        ++start;
    size_t end = command.find(' ', start);
    if (end == std::string::npos)
    {
        boolSendToClient(clientFd, ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
        return;
    }
    tokens.push_back(command.substr(start, end - start));
    start = end + 1;

    while (start < command.length() && command[start] == ' ')
        ++start;
    end = command.find(' ', start);
    std::string channels = command.substr(start, end - start);
    if (channels.empty())
    {
        boolSendToClient(clientFd, ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
        return;
    }

    std::string comment;
    if (end != std::string::npos)
    {
        start = end + 1;
        while (start < command.length() && command[start] == ' ')
            ++start;

        if (start < command.length())
        {
            if (command[start] == ':')
                ++start;
            comment = command.substr(start);
        }
    }

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
    for (size_t i = 0; i < channelList.size(); ++i)
        partUser(clientFd, channelList[i], comment);
}

void Server::partUser(int clientFd, const std::string& channelName, const std::string& comment)
{
    if (channelName.empty())
    {
        boolSendToClient(clientFd, ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
        return;
    }
    std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
    {
        boolSendToClient(clientFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    Channel* channel = it->second;
    if (!channel->isUser(clientFd))
    {
        boolSendToClient(clientFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    std::string partMsg = ":" + _clients[clientFd]->getPrefix() + " PART " + channelName;
    if (!comment.empty())
        partMsg += " :" + comment;
    channel->boolBroadCastToAll(partMsg, this, false);

    channel->removeUser(clientFd);
    channel->removeOperator(clientFd);
    if (!channel->hasOperators() && !channel->getUserFds().empty())
    {
        int newOpFd = *(channel->getUserFds().begin());
        channel->addOperator(newOpFd);

        std::string newOpNick = _clients[newOpFd]->getNickname();
        std::string modeMsg = ":" + _serverName + " MODE " + channel->getName() + " +o " + newOpNick;

        channel->broadcastToAllRaw(modeMsg, this);
    }

    if (channel->getUserFds().empty())
    {
        delete channel;
        _channels.erase(it);
    }
}

void Server::handlePing(int clientFd, const std::vector<std::string>& tokens)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    if (tokens.size() < 2)
    {
        sendReply(clientFd, macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " PING :Not enough parameters");
        return;
    }
    sendReply(clientFd, "PONG " + tokens[1]);
}

void Server::handlePong(int clientFd, const std::vector<std::string>& tokens)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    if (tokens.size() < 2)
    {
        sendReply(clientFd, macroToString(ERR_NEEDMOREPARAMS) + " " + nick + " :Not enough parameters");
        return;
    }
}

void Server::handleQuit(int clientFd, const std::vector<std::string>& tokens)
{
    if (!_clients[clientFd]->isRegistered())
    {
        close(clientFd);
        delete _clients[clientFd];
        _clients.erase(clientFd);
        return;
    }
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    std::string message = "Client quit";
    if (tokens.size() >= 2 && tokens[1][0] == ':')
        message = tokens[1].substr(1);
    else if (tokens.size() >= 2)
        message = tokens[1];
        
    Client* client = _clients[clientFd];
    std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + 
                          client->getHostname() + " QUIT :" + message;
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it !=_channels.end(); ++it)
    {
        Channel* channel = it->second;
        if (channel->isUser(clientFd))
            channel->boolBroadCastToAll(quitMsg, this, false);
    }
    removeClient(clientFd);
}

void Server::joinCommand(int userFd, std::string channelName, std::string key) 
{
    if (channelName == "0")
    {
        std::vector<std::string> channelsToPart;
        for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        {
            if (it->second->isUser(userFd))
                channelsToPart.push_back(it->first);
        }

        for (size_t i = 0; i < channelsToPart.size(); ++i)
        {
            partUser(userFd, channelsToPart[i], "");
        }
        return ;
    }

    if (channelName.empty() || channelName[0] != '#' || channelName == "#")
    {
        boolSendToClient(userFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    Channel* channel = NULL;

    if (_channels.find(channelName) == _channels.end())
    {
        channel = new Channel(channelName);
        if (!key.empty())
            channel->setKey(key);

        _channels.insert(std::make_pair(channelName, channel));
        channel->addUser(userFd, this->_clients[userFd]);
        channel->addOperator(userFd);
    }
    else
    {
        channel = _channels[channelName];

        if (channel->isUser(userFd))
            return;

        if (channel->isInviteOnly() && !channel->isInvited(userFd)) {
            boolSendToClient(userFd, ERR_INVITEONLYCHAN, channelName + " :Cannot join channel (+i)");
            return;
        }

        if (!channel->canJoin(key)){
            boolSendToClient(userFd, ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
            return;
        }

        if (channel->isFull()) {
            boolSendToClient(userFd, ERR_CHANNELISFULL, channelName + " :Cannot join channel (+l)");
            return;
        }

        channel->addUser(userFd, this->_clients[userFd]);
    }

    std::string joinMsg = ":" + _clients[userFd]->getPrefix() + " JOIN :" + channelName;
    channel->boolBroadCastToAll(joinMsg, this, false);

    if (!(channel->getTopic().empty()))
        boolSendToClient(userFd, RPL_TOPIC, channelName + " :" + channel->getTopic());
    else
        boolSendToClient(userFd, RPL_NOTOPIC, channelName + " :No topic is set");

    std::string userList = Server::vecToStr(channel->getNicknamesWithPrefixes());
    boolSendToClient(userFd, RPL_NAMREPLY, "= " + channelName + " :" + userList);
    boolSendToClient(userFd, RPL_ENDOFNAMES, channelName + " :End of /NAMES list");
}



void Server::topicCommand(int userFd, std::string channelName, std::string topic, bool colon)
{
    if (channelName.empty() || channelName[0] != '#')
    {
        sendToClient(userFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    std::map<std::string, Channel*>::iterator it = this->_channels.find(channelName);
    if (it == this->_channels.end())
    {
        sendToClient(userFd, ERR_NOSUCHCHANNEL,  channelName + " :No such channel");
        return ;
    }
    Channel &channel = *(it->second);
    if (!channel.isUser(userFd))
    {
        sendToClient(userFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    if (topic.empty() && !colon)
    {
        const std::string &topic = channel.getTopic();
        if (topic.empty())
            sendToClient(userFd, RPL_NOTOPIC, " " + channel.getName() + " :No topic is set");
        else
            sendToClient(userFd, RPL_TOPIC, " " + channelName + " :" + topic);
        return;
    }
    if (channel.isTopicRestricted() && !channel.isOperator(userFd))
    {
        sendToClient(userFd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator"); 
        return;
    }
    std::string newTopic = topic;
    if (newTopic.empty())
        channel.clearTopic();
    else
        channel.setTopic(newTopic);
    
    std::string msg = ":" + _clients[userFd]->getPrefix() + " TOPIC " + channelName + " :" + newTopic;
    broadcastToAll(channel, msg, -1);

}

void Server::inviteCommand(int senderFd, const std::vector<std::string>& tokens)
{
    if (!_clients[senderFd]->isRegistered())
    {
        sendReply(senderFd, macroToString(ERR_NOTREGISTERED) + " * :You have not registered"); 
        return;
    }
    if (tokens.size() < 3)
    {
        sendError(senderFd, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
        return;
    }
    std::string targetNick = tokens[1];
    std::string channelName = tokens[2];
    
    Client* targetClient = getClientByNickname(targetNick);
    if (!targetClient)
    {
        sendToClient(senderFd, ERR_NOSUCHNICK, targetNick + " :No such nick");
        return;
    }
    if (!_clients[targetClient->getFd()]->isRegistered())
    {
        sendReply(senderFd, macroToString(ERR_NOTREGISTERED) + " * :The user is not registered"); 
        return;
    }
    std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
    {
        sendToClient(senderFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    Channel& channel = *(it->second);
    if (!channel.isUser(senderFd))
    {
        sendToClient(senderFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    if (channel.isInviteOnly() && !channel.isOperator(senderFd))
    {
        sendToClient(senderFd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    int targetFd = targetClient->getFd();
    if (channel.isUser(targetFd))
    {
        sendToClient(senderFd, ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
        return;
    }
    channel.addInvite(targetFd);
    sendToClient(senderFd, RPL_INVITING, targetNick + " " + channelName);
    std::string inviteMsg = ":" + _clients[senderFd]->getPrefix() + 
        " INVITE " + targetNick + " :" + channelName;
    sendReply(targetFd, inviteMsg);
}

void Server::kickCommand(int senderFd, const std::vector<std::string>& tokens)
{
    if (!_clients[senderFd]->isRegistered())
    {
        sendReply(senderFd, macroToString(ERR_NOTREGISTERED) + " * :You have not registered"); 
        return;
    }
    if (tokens.size() < 3)
    {
        sendReply(senderFd, "461 KICK :Not enough parameters");
        return;
    }

    std::vector<std::string> channels = splitByComma(tokens[1]);
    std::vector<std::string> targetNicks = splitByComma(tokens[2]);
    std::string reason = (tokens.size() >= 4) ? tokens[3].substr(1) : _clients[senderFd]->getNickname();

    if (channels.size() != 1 && channels.size() != targetNicks.size())
    {
        sendReply(senderFd, "461 KICK :Channel/Nick list mismatch");
        return;
    }

    for (size_t i = 0; i < targetNicks.size(); ++i)
    {
        std::string channelName = (channels.size() == 1) ? channels[0] : channels[i];
        std::string targetNick = targetNicks[i];

        std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
        if (it == _channels.end())
        {
            sendReply(senderFd, "403 " + channelName + " :No such channel");
            continue;
        }

        Channel* channel = it->second;
        if (!channel->isUser(senderFd))
        {
            sendReply(senderFd, "442 " + channelName + " :You're not on that channel");
            continue;
        }

        if (!channel->isOperator(senderFd))
        {
            sendReply(senderFd, "482 " + channelName + " :You're not channel operator");
            continue;
        }

        Client* targetClient = getClientByNickname(targetNick);
        if (!targetClient)
        {
            sendReply(senderFd, "401 " + targetNick + " :No such nick");
            continue;
        }

        int targetFd = targetClient->getFd();
        if (!channel->isUser(targetFd))
        {
            sendReply(senderFd, "441 " + targetNick + " " + channelName + " :They aren't on that channel");
            continue;
        }

        std::string kickMsg = ":" + _clients[senderFd]->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;

        channel->broadcastToAllRaw(kickMsg, this);
        channel->kickUser(targetFd);
    }
}

void Server::modeCommand(int userFd, const std::vector<std::string>& tokens)
{
    if (!_clients[userFd]->isRegistered())
    {
        sendReply(userFd, macroToString(ERR_NOTREGISTERED) + " * :You have not registered"); 
        return;
    }
    if (tokens.size() < 2)
    {
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return;
    }
    std::string channelName = tokens[1];
    if (channelName.empty())
    {
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return;
    }
	std::map<std::string, Channel*>::iterator it = _channels.find(tokens[1]);
    if (it == _channels.end())
    {
        sendToClient(userFd, ERR_NOSUCHCHANNEL, tokens[1] + " :No such channel");
        return;
    }
    
    Channel& channel = *(it->second);
    if (tokens.size() == 2)
    {
        sendToClient(userFd, RPL_CHANNELMODEIS, channelName + " " + channel.getModeString());
        return ;
    }
	if (!channel.isUser(userFd))
    {
        sendToClient(userFd, ERR_USERONCHANNEL, tokens[1] + " :You're not on that channel");
        return;
    }
	if (!channel.isOperator(userFd))
    {
        sendToClient(userFd, ERR_CHANOPRIVSNEEDED, tokens[1] + " :You're not channel operator");
        return;
    }
	char	sign = tokens[2][0];
	if (sign != '+' && sign != '-')
	{
        sendToClient(userFd, ERR_UNKNOWNMODE, std::string(1, sign) + " :is unknown mode char");
		return ;
	}
    size_t paramIndex = 3;
	for (int i = 1; tokens[2][i]; i++)
	{
		char mode = tokens[2][i];
        std::string param = (paramIndex < tokens.size()) ? tokens[paramIndex] : "";
		switch (mode)
		{
			case 'i':
				channel.setInviteFlag(sign);
				break;
			case 't':
				channel.setRestrictions(sign);
				break;
			case 'k':
			{
				if (sign == '+' && param.empty())
				{
                    sendToClient(userFd, ERR_NEEDMOREPARAMS, "MODE +k :Key required");
					return ;
				}
				channel.setKeyMode(sign, tokens[3]);
                if (sign == '+') paramIndex++;
				break;
			}
			case 'o':
			{
				if (param.empty())
                {
                    sendToClient(userFd, ERR_NEEDMOREPARAMS, "MODE +o :User required");
                    return;
                }
                Client* target = getClientByNickname(param);
                if (!target || !channel.isUser(target->getFd()))
                {
                    sendToClient(userFd, ERR_USERNOTINCHANNEL, param + " " + channelName + " :They aren't on that channel");
                    return;
                }
				channel.setOperatorMode(sign, target->getFd());
                paramIndex++;
				break;
			}
			case 'l':
			{
				if (sign == '+' && param.empty())
				{
					sendToClient(userFd, ERR_NEEDMOREPARAMS, "MODE +l :Limit required");
					return ;
				}
                if (sign == '+')
                {
                    int limit = 0;
                    Server::stringToInt(tokens[3], limit);
                    if (limit < 1)
                    {
                        sendToClient(userFd, ERR_NEEDMOREPARAMS, " MODE :Invalid channel limit (must be at least 1)");
                        return ;
                    }
				    channel.setUserLimit(limit);
                    paramIndex++;
                }
                else
				    channel.setUserLimit(-1);
				break;
			}
			default:
			{
				sendToClient(userFd, ERR_UNKNOWNMODE, std::string(1, mode) + " :is unknown mode char");
                return;
			}
		}
	}
    std::string fullModeStr = tokens[2];
    for (size_t i = 3; i < tokens.size(); ++i)
        fullModeStr += " " + tokens[i];
    channel.broadcastToAll(":" + getClientPrefix(userFd) + " MODE " + tokens[1] + " " + fullModeStr, this);
}
