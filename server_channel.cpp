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

void Server::broadcastToAll(const Channel& channel, const std::string& msg, int excludeFd) // Taha compilation Error
{
    const std::set<int>& users = channel.getUserFds();

    for (std::set<int>::const_iterator it = users.begin(); it != users.end(); ++it) {
        if (*it != excludeFd)
            sendReply(*it, msg);
    }
}

std::string Server::getClientPrefix(int fd) const // Taha fixed
{
    std::map<int, Client*>::const_iterator it = _clients.find(fd);
    if (it != _clients.end())
        return it->second->getPrefix();
    return "";
}


//---------------------------------- COMMANDS ---------------------------------
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
        boolSendToClient(userFd, ERR_NOSUCHCHANNEL, channelName);
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


//--------------------------------------------------------------------------------
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

void Server::topicCommand(int userFd, std::string channelName, std::string topic, bool colon) //commented by Taha compile error!
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

//----------------------------------------------------------------------------------
/* 
    syntax: 
        INVITE <nickname> <channel>
*/

void Server::inviteCommand(int senderFd, const std::vector<std::string>& tokens) // Taha compilation Error
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

    // Notify invitee // commented by Taha compile error
    std::string inviteMsg = ":" + _clients[senderFd]->getPrefix() + 
    " INVITE " + targetNick + " :" + channelName;
    sendReply(targetFd, inviteMsg);
}

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