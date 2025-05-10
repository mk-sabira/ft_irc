#include "Channel.hpp"


/* syntax: 
    INVITE <nickname> <channel>
*/

// void Server::inviteCommand(int senderFd, const std::vector<std::string>& tokens)
void Server::inviteCommand(int senderFd, const std::vector<std::string>& tokens)
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
        sendError(senderFd, ERR_NOSUCHNICK, targetNick + " :No such nick");
        return;
    }

    std::map<std::string, Channel>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
    {
        sendError(senderFd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    Channel& channel = it->second;

    if (!channel.isUser(senderFd))
    {
        sendError(senderFd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel"); // ERR_NOTONCHANNEL
        return;
    }

    if (channel.isInviteOnly() && !channel.isOperator(senderFd))
    {
        sendError(senderFd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }

    int targetFd = targetClient->getFd();

    if (channel.isUser(targetFd))
    {
        sendError(senderFd, ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
        return;
    }

    // Add to channel's invite list
    channel.addInvite(targetFd);

    // Notify inviter
    sendReply(senderFd, "341", targetNick + " " + channelName); // RPL_INVITING

    // Notify invitee
    std::string inviteMsg = ":" + _clients[senderFd]->getPrefix() +
                            " INVITE " + targetNick + " :" + channelName;
    sendMessage(targetFd, inviteMsg);
}
