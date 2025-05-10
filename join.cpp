#include "channel.hpp"

/*
- Check if channel name is valid
- check channel existance
- If user already in the channel, do nothing
- chaeck the [invite-only, key"password", user limit]
- add user to the map
- broadcast JOIN to other users
- send topic and users list to the new user
*/

// void    joinCommand(int userFd, std::string channelName, std::string key)

void joinCommand(int userFd, std::string channelName, std::string key)
{
    if (channelName.empty() || channelName[0] != '#')
    {
        sendError(userFd, ERR_NOSUCHCHANNEL, channelName);
        return;
    }

    Channel* channel = nullptr;
    bool isNewChannel = false;

    if (channels.find(channelName) == channels.end()) // map for channels <channel_name, channel obj>
    {
        channel = new Channel(channelName);

        // Optional: set key if provided
        // if (!key.empty())
        //     channel->setKey(key);

        channels[channelName] = channel;
        isNewChannel = true;

        channel->addUser(userFd, getClientNickname(userFd)); // client functions
        channel->addOperator(userFd);
    }
    else
    {
        channel = channels[channelName];

        if (channel->isUser(userFd))
            return;

        if (channel->isInviteOnly() && !channel->isInvited(userFd)) {
            sendError(userFd, ERR_INVITEONLYCHAN, channelName);
            return;
        }
        if (!canJoin(userFd, key)){
            sendError(userFd, ERR_BADCHANNELKEY, channelName);
            return;
        }
        if (channel->isFull()) {
            sendError(userFd, ERR_CHANNELISFULL, channelName);
            return;
        }
        channel->addUser(userFd);
    }

    // Success: Notify all users in the channel
    // std::string prefix = getClientPrefix(userFd); // nick!user@host
    // std::string joinMsg = ":" + prefix + " JOIN " + channelName;

    // channel->broadcastToAll(joinMsg);

    // Send topic
    // if (!channel->_topic.emprty())
    //     sendToClient(userFd, RPL_TOPIC + " " + channelName + " :" + channel->getTopic());
    // else
    //     sendToClient(userFd, RPL_NOTOPIC + " " + channelName);

    // // Send user list
    // std::string userList = channel->getNicknamesWithPrefixes();
    // sendToClient(userFd, RPL_NAMREPLY + " = " + channelName + " :" + userList);
    // sendToClient(userFd, RPL_ENDOFNAMES + " " + channelName + " :End of /NAMES list.");
}

