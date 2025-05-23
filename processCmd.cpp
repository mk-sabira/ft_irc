/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   processCmd.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 10:59:00 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/23 09:55:39 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cerrno>
#include <cstring>

CommandType Server::getCommandtype (const std::string& command)
{
    if (command == "PASS" || command == "pass") return CMD_PASS;
    if (command == "NICK" || command == "nick") return CMD_NICK;
    if (command == "USER" || command == "user") return CMD_USER;
    if (command == "PING" || command == "ping") return CMD_PING;
    if (command == "PONG" || command == "pong") return CMD_PONG;
    if (command == "PRIVMSG" || command == "privmsg") return CMD_PRIVMSG;
    if (command == "JOIN" || command == "/join") return CMD_JOIN;
    if (command == "TOPIC" || command == "/topic") return CMD_TOPIC;
    if (command == "INVITE" || command == "/invite") return CMD_INVITE;
    if (command == "KICK" || command == "/kick") return CMD_KICK;
    if (command == "MODE" || command == "/mode") return CMD_MODE;
    if (command == "QUIT" || command == "quit") return CMD_QUIT;
    return CMD_UNKNOWN;
}

void Server::splitCommand(std::vector<std::string>& tokens, const std::string& command, std::string::size_type start, std::string::size_type end)
{
    tokens.push_back(command.substr(start, end - start)); // Command name
    start = end + 1;

    while (start < command.length())
    {
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

void Server::handlePass(int clientFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2 || tokens[1].empty())
    {
        sendReply(clientFd, " 461 PASS :Not enough parameters");
        return ;
    }
    if (_clients[clientFd]->isAuthenticated())
    {
        sendReply(clientFd, " 462 :Unauthorized command (already registered)");
        return ;
    }
    if (tokens[1] != _password)
    {
        sendReply(clientFd, " 464 * :Password incorrect");
        removeClient(clientFd);
        return ;
    }
    _clients[clientFd]->setAuthenticated(true);
    // std::cout << "Client FD " << clientFd << " authenticated" << std::endl;
}

bool Server::validateNick(const std::string& nick, std::string& errorMsg)
{
    // std::cout<< nick << ":size " << nick.size() << std::endl;
    if (nick.empty() || nick.length() > 9)
    {
        errorMsg = "432 " + nick + " :Erroneous nickname";
        return false;
    }
    if (!isalpha(nick[0]) && nick[0] != '[' && nick[0] != ']' && nick[0] != '\\' && 
        nick[0] != '`' && nick[0] != '_' && nick[0] != '^' && nick[0] != '{' && 
        nick[0] != '|' && nick[0] != '}')
    {
        errorMsg = "432 " + nick + " :Erroneous nickname";
        return false;
    }
    for (size_t i = 1; i < nick.length(); ++i)
    {
        if (!isalnum(nick[i]) && nick[i] != '-' && nick[i] != '[' && nick[i] != ']' && 
            nick[i] != '\\' && nick[i] != '`' && nick[i] != '_' && nick[i] != '^' && 
            nick[i] != '{' && nick[i] != '|' && nick[i] != '}')
        {
            errorMsg = "432 " + nick + " :Erroneous nickname";
            return false;
        }
    }
    return true;
}

void Server::handleNick(int clientFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2 || tokens[1].empty())
    {
        sendReply(clientFd, "431 :No nickname given");
        return;
    }
    std::string errorMess;
    if ( tokens.size() != 2)
    {
        sendReply(clientFd, "432 " " :Erroneous nickname");
        return ;
    }
    if (!validateNick(tokens[1], errorMess))
    {
        sendReply(clientFd, errorMess);
        return ;
    }
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->first != clientFd && it->second->getNickname() == tokens[1])
        {
            sendReply(clientFd, "433 " + tokens[1] + " :Nickname is already in use");
            return;
        }
    }
    _clients[clientFd]->setNickname(tokens[1]);
    if (_clients[clientFd]->isAuthenticated() && !_clients[clientFd]->getNickname().empty() && !_clients[clientFd]->getUsername().empty())
    {
        _clients[clientFd]->setRegistered(true);
        sendReply(clientFd, "001 " + _clients[clientFd]->getNickname() + " :Welcome to the IRC server");
        sendReply(clientFd, "002 " + _clients[clientFd]->getNickname() + " :Your host is " + _serverName);
        sendReply(clientFd, "003 " + _clients[clientFd]->getNickname() + " :This server was created today");
        sendReply(clientFd, "004 " + _clients[clientFd]->getNickname() + " :" + _serverName + " 1.0");
    }
}

bool Server::validateUser(const std::vector<std::string>& tokens, std::string& errorMsg)
{
    if (tokens.size() < 5)
    {
        errorMsg = "461 USER :Not enough parameters";
        return false;
    }
    const std::string& user = tokens[1];
    const std::string& mode = tokens[2];
    const std::string& realname = tokens[4];

    if (user.empty() || user.find(' ') != std::string::npos)
    {
        errorMsg = "461 USER :Not enough parameters";
        return false;
    }
    for (size_t i = 0; i < user.length(); ++i)
    {
        if (!isalnum(user[i]) && user[i] != '_' && user[i] != '-')
        {
            errorMsg = "461 USER :Not enough parameters";
            return false;
        }
    }

    // Validate mode: numeric, 0, 4, 8, or 12 //discover more about it
    int modeVal = 0;
    try{
        // modeVal = std::stoi(mode);
        stringToInt(mode, modeVal);
    }
    catch (...)
    { 
        errorMsg = "461 USER :Not enough parameters"; 
        return false; 
    }
    if (modeVal != 0 && modeVal != 4 && modeVal != 8 && modeVal != 12)
    {
        errorMsg = "461 USER :Not enough parameters";
        return false;
    }

    if (realname.empty())
    {
        errorMsg = "461 USER :Not enough parameters";
        return false;
    }
    if (realname.find(' ') != std::string::npos && realname[0] != ':')
    {
        errorMsg = "461 USER :Not enough parameters";
        return false;
    }

    return true;
}

void Server::handleUser(int clientFd, const std::vector<std::string>& tokens)
{
    if (_clients[clientFd]->isRegistered())
    {
        sendToClient(clientFd, ERR_ALREADYREGISTRED, ":Unauthorized command (already registered)");
        return;
    }
    std::string errorMsg;
    if (!validateUser(tokens, errorMsg))
    {
        sendReply(clientFd, errorMsg);
        return;
    }
    _clients[clientFd]->setUsername(tokens[1]);
    _clients[clientFd]->setRealname(tokens[4]);
    if (_clients[clientFd]->isAuthenticated() && !_clients[clientFd]->getNickname().empty() && !_clients[clientFd]->getUsername().empty())
    {
        _clients[clientFd]->setRegistered(true);
        sendReply(clientFd, "001 " + _clients[clientFd]->getNickname() + " :Welcome to the IRC server");
        sendReply(clientFd, "002 " + _clients[clientFd]->getNickname() + " :Your host is " + _serverName);
        sendReply(clientFd, "003 " + _clients[clientFd]->getNickname() + " :This server was created today");
        sendReply(clientFd, "004 " + _clients[clientFd]->getNickname() + " :" + _serverName + " 1.0");
        std::cout << CYAN << "New client connected: FD = " << clientFd << RESET << std::endl;
    }

}

void Server::sendReply(int clientFd, const std::string& message)
{
    std::string msg = ":" + _serverName + " " + message + "\r\n";
    int bytesSent = send(clientFd, msg.c_str(), msg.length(), 0);
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            std::cout << "Send to FD " << clientFd << " blocked, will retry" << std::endl;
        else
            std::cerr << "Error sending to FD " << clientFd << ": " << strerror(errno) << std::endl;
    }
    else if (bytesSent != static_cast<int>(msg.length()))
    {
        std::cout << "Partial send to FD " << clientFd << ": " << bytesSent << "/" << msg.length() << " bytes" << std::endl;
    }
}

void Server::handlePrivmsg(int senderFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 3 || tokens[2].empty())
    {
        sendReply(senderFd, "412 :No text to send");
        return;
    }

    std::string targetNick = tokens[1];
    std::string message = tokens[2];

    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == targetNick)
        {
            std::string msgToSend = ":" + _clients[senderFd]->getNickname() + " PRIVMSG " + targetNick + " :" + message + "\r\n";
            send(it->first, msgToSend.c_str(), msgToSend.size(), 0);
            return;
        }
    }

    sendReply(senderFd, "401 " + targetNick + " :No such nick/channel");
}

void Server::handlePing(int clientFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2)
    {
        sendReply(clientFd, "461 PING :Not enough parameters");
        return;
    }
    sendReply(clientFd, "PONG " + tokens[1]);
}

void Server::handlePong(int clientFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2)
    {
        sendReply(clientFd, "461 PONG :Not enough parameters");
        return;
    }
}

//-------------------- CHANNEL COMMANDS --------------------------------

void Server::parseJoinCommand(int userFd, const std::string& command)
{
    std::vector<std::string> tokens;
    size_t                   start = 0;
    size_t                   end = command.find(' ');

    while (end != std::string::npos && command[start] == ' ')
    {
        ++start;
        end = command.find(' ', start);
    }

    if (end == std::string::npos)
        return;

    tokens.push_back(command.substr(start, end - start));
    start = end + 1;

    while (start < command.length() && command[start] == ' ')
        ++start;

    end = command.find(' ', start);
    std::string channels = command.substr(start, end - start);

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

void Server::joinCommand(int userFd, std::string channelName, std::string key) 

{
    if (channelName == "0")
    {
        for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); )
        {
            Channel* channel = it->second;
            if (channel->isUser(userFd)) {
                channel->kickUser(userFd);

                std::string partMsg = ":" + _clients[userFd]->getPrefix() + " PART " + channel->getName();
                channel->boolBroadCastToAll(partMsg, this, false); // Use user prefix

                if (channel->getUserFds().empty()) {
                    delete channel;
                    _channels.erase(it++);
                    continue;
                }
            }
            ++it;
        }
        return;
    }

    if (channelName.empty() || channelName[0] != '#')
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
    channel->boolBroadCastToAll(joinMsg, this, false);  // Don't prepend server name

    if (!(channel->getTopic().empty()))
        boolSendToClient(userFd, RPL_TOPIC, channelName + " :" + channel->getTopic());
    else
        boolSendToClient(userFd, RPL_NOTOPIC, channelName + " :No topic is set");

    std::string userList = vecToStr(channel->getNicknamesWithPrefixes());
    boolSendToClient(userFd, RPL_NAMREPLY, "= " + channelName + " :" + userList);
    boolSendToClient(userFd, RPL_ENDOFNAMES, channelName + " :End of /NAMES list");
}

void Server::parseTopicCommand( int userFd, const std::string& command)
{
    std::vector<std::string> tokens;
    std::string::size_type   start = 0;
    std::string::size_type   end = command.find(' ');

    // Skip any leading spaces after the command name
    while (end != std::string::npos && command[start] == ' ')
    {
        ++start;
        end = command.find(' ', start);
    }
     if (command == "/topic")
     {
         // Missing channel parameter
         sendToClient(userFd, ERR_NEEDMOREPARAMS, "TOPIC: Not enough parameters");
         return;
     }
    if (end == std::string::npos)
        return;

    tokens.push_back(command.substr(start, end - start)); // Command name (e.g., "TOPIC")
    start = end + 1;

    // Skip additional spaces
    while (start < command.length() && command[start] == ' ')
        ++start;

    if (start >= command.length())
    {
        // Missing channel parameter
        sendToClient(userFd, ERR_NEEDMOREPARAMS, "TOPIC: Not enough parameters");
        return;
    }

    // Extract channel
    end = command.find(' ', start);
    std::string channel = command.substr(start, end - start);
    start = (end == std::string::npos) ? std::string::npos : end + 1;

    // Skip additional spaces before topic (if present)
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

void Server::topicCommand(int userFd, std::string channelName, std::string topic, bool colon) //commented by Taha compile error! // fixed
{
    if (channelName.empty() || channelName[0] != '#')
    {
        sendToClient(userFd, ERR_NOSUCHCHANNEL, channelName);
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
    std::string newTopic = topic; // assumes already parsed with ":" removed
    if (newTopic.empty())
        channel.clearTopic();
    else
        channel.setTopic(newTopic);
    
    std::string msg = ":" + _clients[userFd]->getPrefix() + " TOPIC " + channelName + " :" + newTopic;
    broadcastToAll(channel, msg, -1);

}

void Server::inviteCommand(int senderFd, const std::vector<std::string>& tokens) // Taha compilation Error //fixed
{
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

    std::map<std::string, Channel*>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
    {
        sendToClient(senderFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    Channel& channel = *(it->second);

    if (!channel.isUser(senderFd))
    {
        sendToClient(senderFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel"); // ERR_NOTONCHANNEL
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

    // Notify inviter
    sendToClient(senderFd, RPL_INVITING, targetNick + " " + channelName); //invite reply

    // Notify invitee // commented by Taha compile error // fixed
    std::string inviteMsg = ":" + _clients[senderFd]->getPrefix() + 
    " INVITE " + targetNick + " :" + channelName;
    sendReply(targetFd, inviteMsg);
}

void Server::kickCommand(int senderFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 3)
    {
        sendReply(senderFd, "461 KICK :Not enough parameters");
        return;
    }

    std::string channelName = tokens[1];
    std::string targetNick = tokens[2];
    std::string reason = (tokens.size() >= 4) ? tokens[3].substr(1) : _clients[senderFd]->getNickname();

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

    std::string msg = ":" + _clients[senderFd]->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
    channel->broadcastToAll(msg, this);
    channel->kickUser(targetFd);
}

void Server::modeCommand(int userFd, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3)
	{
		// return the current modes.
		return ;

		/*
			Code		324
			Name		RPL_CHANNELMODEIS
			Description	Returned when a client requests to view the current modes of a channel.
			Format:		 :server 324 <nick> <channel> <modes> [params]
		*/
	}
	std::map<std::string, Channel*>::iterator it = _channels.find(tokens[1]);
    if (it == _channels.end())
    {
        sendToClient(userFd, ERR_NOSUCHCHANNEL, tokens[1] + " :No such channel");
        return;
    }

    Channel& channel = *(it->second);
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
		//error 
		return ;
	}
	for (int i = 1; tokens[2][i]; i++)
	{
		char mode = tokens[2][i];
		switch (mode)
		{
			case 'i':
			{
				channel.setInviteFlag(sign);
				break;
			}
			case 't':
				channel.setRestrictions(sign);
				break;
			case 'k':
			{
				if (tokens.size() < 4 && sign == '+')
				{
					//error
					return ;
				}
				channel.setKeyMode(sign, tokens[3]);
				break;
			}
			case 'o':
			{
				if (tokens.size() < 4)
				{
					//error
					return ;
				}
				channel.setOperatorMode(sign, getClientByNickname(tokens[3])->getFd());
				break;
			}
			case 'l':
			{
				if (tokens.size() < 4 && sign == '+')
				{
					//error
					return ;
				}
                if (sign == '+')
                {
                    int limit = 0;
                    stringToInt(tokens[3], limit);
				    channel.setUserLimit(sign, limit);
                }
                else
				    channel.setUserLimit(sign, -1);
				break;
			}
		
			default:
			{
				//error
				return ;
				// break;
			}
		}

	}


}

void Server::boolSendReply(int clientFd, const std::string& message, bool useServerPrefix)
{
    std::string msg;
    if (useServerPrefix)
        msg = ":" + _serverName + " " + message + "\r\n";
    else
        msg = message + "\r\n";

    int bytesSent = send(clientFd, msg.c_str(), msg.length(), 0);
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            std::cout << "Send to FD " << clientFd << " blocked, will retry" << std::endl;
        else
            std::cerr << "Error sending to FD " << clientFd << ": " << strerror(errno) << std::endl;
    }
    else if (bytesSent != static_cast<int>(msg.length()))
    {
        std::cout << "Partial send to FD " << clientFd << ": " << bytesSent << "/" << msg.length() << " bytes" << std::endl;
    }
}