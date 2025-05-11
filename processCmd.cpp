/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   processCmd.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 10:59:00 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/11 14:05:00 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::splitCommand(std::vector<std::string>& tokens, const std::string& command, std::string::size_type start, std::string::size_type end )
{
    tokens.push_back(command.substr(start, end - start)); // Command name
    start = end + 1;
    if (tokens[0] == "USER")
    {
        // Split first three parameters
        for (int i = 0; i < 3 && end != std::string::npos; ++i)
        {
            end = command.find(' ', start);
            if (end == std::string::npos)
                break;
            tokens.push_back(command.substr(start, end - start));
            start = end + 1;
        }
        // Take rest as realname
        if (start < command.length())
            tokens.push_back(command.substr(start));
    }
    else if (tokens[0] == "PRIVMSG")
    {
        end = command.find(' ', start);
        if (end != std::string::npos)
        {
            tokens.push_back(command.substr(start, end - start)); // Target
            start = end + 1;
            if (start < command.length())
                tokens.push_back(command.substr(start)); // Message (includes ':')
        }
    }
    else
    {
        // Split remaining parameters
        while (end != std::string::npos)
        {
            end = command.find(' ', start);
            tokens.push_back(command.substr(start, end == std::string::npos ? end : end - start));
            start = end + 1;
        }
    }
}

void Server::handlePass(int clientFd, const std::vector<std::string>& tokens)
{
    if (tokens.size() < 2)
    {
        sendReply(clientFd, " 461 PASS :Not enough parameters");
        return ;
    }
    if (_clients[clientFd].isAuthenticated())
    {
        sendReply(clientFd, " 462 :You may not reregister");
        return ;
    }
    if (tokens[1] != _password)
    {
        sendReply(clientFd, " 464 :Password incorrect");
        removeClient(clientFd);
        return ;
    }
    _clients[clientFd].setAuthenticated(true);
    std::cout << "Client FD " << clientFd << " authenticated" << std::endl;
}




void Server::handleNick(int clientFd, const std::vector<std::string>& tokens)
{
    if (!_clients[clientFd].isAuthenticated())
    {
        sendReply(clientFd, "451 :You have not registered");
        return;
    }
    if (tokens.size() < 2 || tokens[1].empty())
    {
        sendReply(clientFd, "431 :No nickname given");
        return;
    }
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->first != clientFd && it->second.getNickname() == tokens[1])
        {
            sendReply(clientFd, "433 " + tokens[1] + " :Nickname is already in use");
            return;
        }
    }
    _clients[clientFd].setNickname(tokens[1]);
    if (_clients[clientFd].isAuthenticated() && !_clients[clientFd].getNickname().empty() && !_clients[clientFd].getUsername().empty())
    {
        _clients[clientFd].setRegistered(true);
        sendReply(clientFd, "001 " + _clients[clientFd].getNickname() + " :Welcome to the IRC server");
        sendReply(clientFd, "002 " + _clients[clientFd].getNickname() + " :Your host is " + _serverName);
        sendReply(clientFd, "003 " + _clients[clientFd].getNickname() + " :This server was created today");
        sendReply(clientFd, "004 " + _clients[clientFd].getNickname() + " :" + _serverName + " 1.0");
    }
}


void Server::handleUser(int clientFd, const std::vector<std::string>& tokens)
{
    if (!_clients[clientFd].isAuthenticated())
    {
        sendReply(clientFd, "451 :You have not registered");
        return;
    }
    if (tokens.size() < 5)
    {
        sendReply(clientFd, "461 USER :Not enough parameters");
        return;
    }
    if (!_clients[clientFd].getUsername().empty())
    {
        sendReply(clientFd, "462 :You may not reregister");
        return;
    }
    _clients[clientFd].setUsername(tokens[1]);
    _clients[clientFd].setRealname(tokens[4]);
    if (_clients[clientFd].isAuthenticated() && !_clients[clientFd].getNickname().empty() && !_clients[clientFd].getUsername().empty())
    {
        _clients[clientFd].setRegistered(true);
        sendReply(clientFd, "001 " + _clients[clientFd].getNickname() + " :Welcome to the IRC server");
        sendReply(clientFd, "002 " + _clients[clientFd].getNickname() + " :Your host is " + _serverName);
        sendReply(clientFd, "003 " + _clients[clientFd].getNickname() + " :This server was created today");
        sendReply(clientFd, "004 " + _clients[clientFd].getNickname() + " :" + _serverName + " 1.0");
    }
}

// void Server::sendReply(int clientFd, const std::string& message)
// {
//     std::string msg = message + "\r\n";
//     if (send(clientFd, msg.c_str(), msg.length(), 0) < 0)
//         std::cerr << "Error sending to FD " << clientFd << ": " << strerror(errno) << std::endl;
// }

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

void Server::handlePrivmsg(int clientFd, const std::vector<std::string> &tokens)
{
    if (!_clients[clientFd].isAuthenticated())
    {
        sendReply(clientFd, "451 :You have not registered");
        return;
    }
    if (tokens.size() < 3 || tokens[2].empty() || tokens[2][0] != ':')
    {
        sendReply(clientFd, "461 PRIVMSG :Not enough parameters");
        return;
    }
    std::string target = tokens[1];
    if (target[0] == '#')
    {
        sendReply(clientFd, "403 " + target + " :No such channel");
        return; // Channels not ready
    }
    std::string message = tokens[2].substr(1); // Remove leading ':'
    std::string prefix = ":" + _clients[clientFd].getNickname() + "!" + _clients[clientFd].getUsername() + "@localhost";
    
    int targetFd = -1;
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == target)
        {
            targetFd = it->first;
            break;
        }
    }
    if (targetFd == -1)
    {
        sendReply(clientFd, "401 " + target + " :No such nick");
        return;
    }
    sendReply(targetFd, prefix + " PRIVMSG " + target + " :" + message);
    std::cout << "Sent PRIVMSG from FD " << clientFd << " to FD " << BOLD_CYAN << targetFd << ": " << message << RESET <<std::endl;
}
