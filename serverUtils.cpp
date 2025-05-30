/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 10:50:44 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/30 13:04:23 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::welcomeMessage()
{
    std::cout << GREEN
          << "    ╔══════════════════════════════════════════════════════╗\n"
          << "    ║                                                      ║\n"
          << "    ║     ██████╗ ████████╗     ██████╗  ███████╗ ███████  ║\n"
          << "    ║    ██╔════╝    ██╔══╗       ██╔═══ ██╗  ██  ██   ██  ║\n"
          << "    ║    ██║███     ╗██           ██║    ██ ██    ██       ║\n"
          << "    ║    ██║         ██║          ██║    ██   ██  ██   ██  ║\n"
          << "    ║    ██║         ██         ██████   ██   ██  ███████  ║\n"
          << "    ║    ╚═╝      ╚═════╝██████╚══════╝          ╚══════╝  ║\n"
          << "    ║                                                      ║\n"
          << "    ║     💬IRC Server is now live and listening           ║\n"
          << "    ║                                                      ║\n"
          << "    ║                                                      ║\n"
          << "    ╚══════════════════════════════════════════════════════╝\n"
          << RESET << std::endl;
}

void Server::notifyChangeNick(int clientFd, const std::string& oldNick, const std::string& newNick)
{
    Client* client = _clients[clientFd];
    std::string user = client->getUsername();
    std::string host = client->getHostname();
    std::string nickMsg = ":" + (oldNick.empty() || oldNick == "*" ? newNick : oldNick) + "!" + user + "@" + host + " NICK :" + newNick + "\r\n";
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        Channel* channel = it->second;
        if (channel->isUser(clientFd))
        {
            std::set<int> memberFds = channel->getUserFds();
            for (std::set<int>::iterator memIt = memberFds.begin(); memIt != memberFds.end(); ++memIt)
            {
                int fd = *memIt;
                sendRaw(fd, nickMsg);
            }
        }
    }
    sendRaw(clientFd, nickMsg);
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

void Server::sendRaw(int clientFd, const std::string& rawMessage)
{
    std::string msg = rawMessage + "\r\n";
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

void Server::broadcastToAll(const Channel& channel, const std::string& msg, int excludeFd) // Taha compilation Error
{
    const std::set<int>& users = channel.getUserFds();

    for (std::set<int>::const_iterator it = users.begin(); it != users.end(); ++it) {
        if (*it != excludeFd)
            sendReply(*it, msg);
    }
}

// -------------- UTILS ----------------

void Server::sendError(int userFd, int errorCode, const std::string& message)
{
    std::string nickname = _clients[userFd]->getNickname();
    std::stringstream ss;
    ss  << errorCode << " "
       << (nickname.empty() ? "*" : nickname) << " "
       << message;
    sendReply(userFd, ss.str());
}

void Server::sendToClient(int fd, int code, const std::string& message)
{
    std::stringstream ss;
    ss << code << " " << _clients[fd]->getNickname() << " " << message;
    sendReply(fd, ss.str());
}

void Server::boolSendToClient(int fd, int code, const std::string& message)
{
    std::stringstream ss;
    ss << code << " " << _clients[fd]->getNickname() << " " << message;
    boolSendReply(fd, ss.str(), true);
}

// --------- CLEANING UTILS ----------------

void Server::removeClient(int clientFd)
{
    close(clientFd);
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); )
    {
        Channel* channel = it->second;
        if (channel->isUser(clientFd))
        {
            channel->removeUser(clientFd);
            channel->removeOperator(clientFd);

            if (channel->getUserFds().empty())
            {
                delete channel;
                _channels.erase(it++);
                continue;
            }
            else if (!channel->hasOperators())
            {
                int newOpFd = *(channel->getUserFds().begin());
                channel->addOperator(newOpFd);
        
                std::string newOpNick = _clients[newOpFd]->getNickname();
                std::string modeMsg = ":" + _serverName + " MODE " + channel->getName() + " +o " + newOpNick;
        
                channel->broadcastToAllRaw(modeMsg, this);
            }
            
        }
        ++it;
    }
    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == clientFd)
        {
            _fds.erase(it);
            break;
        }
    }

    std::map<int, Client*>::iterator clientIt = _clients.find(clientFd);
    if (clientIt != _clients.end())
    {
        delete clientIt->second;
        _clients.erase(clientIt);
    }
    std::cout << "Client FD " << clientFd << RED << " disconnected!" << RESET << std::endl;
}

// ------------ helper functions ----------

std::vector<std::string> Server:: splitByComma(const std::string& str)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ','))
        result.push_back(item);

    return result;
}

std::string Server::vecToStr(std::vector<std::string> vec)
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

bool Server::stringToInt(const std::string& str, int& result)
{
    std::istringstream iss(str);
    iss >> result; 

    if (iss.fail() || !iss.eof())
        return false; 

    return true; 
}

std::string Server::macroToString(int macro)
{
    std::ostringstream numeric;
    numeric << macro;
    return (numeric.str());

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


void Server::shutdown()
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        sendReply(it->first, "ERROR :Server shutting down");
        close(it->first);
        delete it->second;
    }
    _clients.clear();
    if (_serverFd != -1)
    {
        close(_serverFd);
        _serverFd = -1;
    }
    _fds.clear();
    shutdownMessage();
}

void Server::shutdownMessage()
{
    std::cout << GREEN
              << "    ╔══════════════════════════════════════════════════════╗\n"
              << "    ║                                                      ║\n"
              << "    ║            💤 FT_IRC Server is shutting down...      ║\n"
              << "    ║                                                      ║\n"
              << "    ║     Thank you for chatting with us!                  ║\n"
              << "    ║     See you next time! 👋                            ║\n"
              << "    ║                                                      ║\n"
              << "    ╚══════════════════════════════════════════════════════╝\n"
              << RESET << std::endl;
}
