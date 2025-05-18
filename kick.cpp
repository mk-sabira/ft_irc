// #include "Channel.hpp" // commented by Dina
// #include <vector>

// /* syntax:
//     KICK <channel>{,<channel>} <user>{,<user>} [<comment>]
// */

// void Server::kickCommand(int senderFd, const std::vector<std::string>& tokens)
// {
//     if (tokens.size() < 3)
//     {
//         sendError(senderFd, ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
//         return;
//     }

//     // std::vector<std::string> channels = split(tokens[1], ',');
//     // std::vector<std::string> users = split(tokens[2], ',');
//     std::string comment;

//     if (tokens.size() > 3)
//         comment = tokens[3];
//     else
//         comment = _clients[senderFd]->getNickname();

//     if (channels.size() == 1)
//     {
//         std::map<std::string, Channel*>::iterator chanIt = _channels.find(channels[0]);
//         if (chanIt == _channels.end())
//         {
//             sendError(senderFd, ERR_NOSUCHCHANNEL, channels[0] + " :No such channel");
//             return;
//         }

//         Channel& channel = *(chanIt->second);

//         if (!channel.isUser(senderFd))
//         {
//             sendError(senderFd, ERR_NOTONCHANNEL, channels[0] + " :You're not on that channel");
//             return;
//         }

//         if (!channel.isOperator(senderFd))
//         {
//             sendError(senderFd, ERR_CHANOPRIVSNEEDED, channels[0] + " :You're not channel operator");
//             return;
//         }

//         for (size_t i = 0; i < users.size(); ++i)
//         {
//             Client* targetClient = getClientByNickname(users[i]);
//             if (!targetClient)
//             {
//                 sendError(senderFd, ERR_NOSUCHNICK, users[i] + " :No such nick"); 
//                 continue;
//             }

//             int targetFd = targetClient->getFd();

//             if (!channel.isUser(targetFd))
//             {
//                 sendError(senderFd, ERR_USERNOTINCHANNEL, users[i] + " " + channels[0] + " :They aren't on that channel");
//                 continue;
//             }

//             // Format the KICK message
//             std::string kickMsg = ":" + _clients[senderFd]->getPrefix() +
//                                   " KICK " + channels[0] + " " + users[i] +
//                                   " :" + comment;

//             // Send KICK to all channel members
//             broadcastToChannel(channel, kickMsg);

//             // Remove user from channel
//             channel.removeUser(targetFd);

//             // Optionally notify kicked user (if not handled in broadcast)
//             sendMessage(targetFd, kickMsg);
//         }
//     }
//     else if (channels.size() == users.size())
//     {
//         // Handle one-to-one kicks
//         for (size_t i = 0; i < channels.size(); ++i)
//         {
//             std::map<std::string, Channel>::iterator chanIt = _channels.find(channels[i]);
//             if (chanIt == _channels.end())
//             {
//                 sendError(senderFd, "403", channels[i] + " :No such channel"); // ERR_NOSUCHCHANNEL
//                 continue;
//             }

//             Channel& channel = chanIt->second;

//             if (!channel.isUserInChannel(senderFd))
//             {
//                 sendError(senderFd, "442", channels[i] + " :You're not on that channel");
//                 continue;
//             }

//             if (!channel.isOperator(senderFd))
//             {
//                 sendError(senderFd, "482", channels[i] + " :You're not channel operator");
//                 continue;
//             }

//             Client* targetClient = getClientByNickname(users[i]);
//             if (!targetClient)
//             {
//                 sendError(senderFd, "401", users[i] + " :No such nick");
//                 continue;
//             }

//             int targetFd = targetClient->getFd();

//             if (!channel.isUserInChannel(targetFd))
//             {
//                 sendError(senderFd, "441", users[i] + " " + channels[i] + " :They aren't on that channel");
//                 continue;
//             }

//             std::string kickMsg = ":" + _clients[senderFd]->getPrefix() +
//                                   " KICK " + channels[i] + " " + users[i] +
//                                   " :" + comment;

//             broadcastToChannel(channel, kickMsg);
//             channel.removeUser(targetFd);
//             sendMessage(targetFd, kickMsg);
//         }
//     }
//     else
//     {
//         sendError(senderFd, "461", "KICK :Mismatched number of channels and users");
//     }
// }

