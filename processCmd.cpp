/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   processCmd.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrhelmy <mrhelmy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 10:59:00 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/17 23:20:12 by mrhelmy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cerrno>
#include <cstring>

// void Server::splitCommand(std::vector<std::string>& tokens, const std::string& command, std::string::size_type start, std::string::size_type end )
// {
//     tokens.push_back(command.substr(start, end - start)); // Command name
//     start = end + 1;
//     if (tokens[0] == "USER")
//     {
//         // Split first three parameters
//         for (int i = 0; i < 3 && end != std::string::npos; ++i)
//         {
//             end = command.find(' ', start);
//             if (end == std::string::npos)
//                 break;
//             tokens.push_back(command.substr(start, end - start));
//             start = end + 1;
//         }
//         // Take rest as realname
//         if (start < command.length())
//             tokens.push_back(command.substr(start));
//     }
//     else if (tokens[0] == "PRIVMSG")
//     {
//         end = command.find(' ', start);
//         if (end != std::string::npos)
//         {
//             tokens.push_back(command.substr(start, end - start)); // Target
//             start = end + 1;
//             if (start < command.length())
//                 tokens.push_back(command.substr(start)); // Message (includes ':')
//         }
//     }
//     else
//     {
//         // Split remaining parameters
//         while (end != std::string::npos)
//         {
//             end = command.find(' ', start);
//             tokens.push_back(command.substr(start, end == std::string::npos ? end : end - start));
//             start = end + 1;
//         }
//     }
// }



bool stringToInt(const std::string& str, int& result)
{
    std::istringstream iss(str);  // Create a string stream from the input string
    iss >> result;                // Try to extract an int from the string

    // Check for conversion failure or extra characters
    if (iss.fail() || !iss.eof())
        return false;             // Conversion failed or leftover characters exist

    return true;                  // Successful conversion
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
        sendReply(clientFd, " 464 :Password incorrect");
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
    std::cout << "Sent PONG to FD " << clientFd << " for PING " << tokens[1] << std::endl;
}
