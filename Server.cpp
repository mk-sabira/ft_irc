/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:25:46 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/25 12:23:44 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cerrno>
#include <arpa/inet.h>

volatile sig_atomic_t Server::keepRunning = 1;

Server::~Server()
{
    // Delete all Clients
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();

    // Delete all Channels
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();

    if (_serverFd != -1)
        close(_serverFd);
}


Server::Server(const std::string &port, const std::string &password):_serverFd(-1), _serverName("irc.server")
{
    std::istringstream ss(port);
    ss >> _port;
    setPort(_port);
    if (getPort() < 1024 || getPort() > 65535)
        throw PortOutOfBound();
    _password = password;
}

bool Server::serverSetup()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd == -1)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return (false);
    }
    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Setsockopt failed" << std::endl;
        close(_serverFd);
        return false;
    }
    if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "Failed to set non-blocking mode" << std::endl;
        close(_serverFd);
        return (false);
    }
    _serverAdd.sin_family = AF_INET;
    _serverAdd.sin_addr.s_addr = INADDR_ANY;
    _serverAdd.sin_port = htons(_port);
    
    if (bind(_serverFd, (struct sockaddr*)& _serverAdd, sizeof(_serverAdd)) < 0)
    {
        std::cout << "Bind failed" << std::endl;
        return (false);
    }

    if (listen(_serverFd, SOMAXCONN) == -1)
    {
        std::cout << "Listen failed" << std::endl;
        return (false);
    }
    
    struct pollfd serverPollFd;
    serverPollFd.fd = _serverFd;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;
    _fds.push_back(serverPollFd);
    
    return (true);
}

bool Server::runServer()
{
    std::cout << GREEN << "IRC Server is running" << RESET << std::endl;

    // Add stdin to poll loop for Ctrl+D
    struct pollfd stdinFd;
    stdinFd.fd = 0; // STDIN_FILENO
    stdinFd.events = POLLIN;
    stdinFd.revents = 0;
    _fds.push_back(stdinFd);
    
    while (Server::keepRunning)
    {
        int pollCount = poll(&_fds[0], _fds.size(), -1);
        if (pollCount == -1)
        {
            if (errno == EINTR)
                break;
            std::cerr << "poll() failed" << strerror(errno) << std::endl;
            shutdown();
            return (false);
            
        }
        for (size_t i = 0; i < _fds.size(); ++i)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_fds[i].fd == 0) // stdin (Ctrl+D)
                {
                    char buf[1];
                    ssize_t bytes = read(0, buf, 1);
                    if (bytes == 0) // EOF (Ctrl+D)
                    {
                        std::cout << "Received Ctrl+D, shutting down server..." << std::endl;
                        Server::keepRunning = 0;
                        break;
                    }
                }
                else if (_fds[i].fd == _serverFd)
                    acceptNewClient();
                else
                    receiveClientData(_fds[i].fd);
            }
            _fds[i].revents = 0;
        }
        if(!Server::keepRunning)
            break;
    }
    shutdown();
    return (true);
}

void Server::acceptNewClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientFd = accept(_serverFd, (struct sockaddr*)& clientAddr, &clientLen);
    if (clientFd == -1)
    {
        std::cerr << "Failed to accept new clinet" << std::endl;
        return ;
    }

    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "Failed to set client socket to non-blocking" << std::endl;
        close(clientFd);
        return ;
    }

    struct pollfd clientPollFd;
    clientPollFd.fd = clientFd;
    clientPollFd.events = POLLIN;
    clientPollFd.revents = 0;

    _fds.push_back(clientPollFd);

    Client* client = new Client();
    client->setFd(clientFd);
    client->setAuthenticated(false);
    client->setHostname(inet_ntoa(clientAddr.sin_addr)); // taha trying to fix lime chat
    _clients[clientFd] = client;
}

void Server::receiveClientData(int clientFd)
{
    int bytesRead;
    char buffer[1024];

    bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            std::cout << "No data on FD " << clientFd << " (EAGAIN)" << std::endl;   
            return ;
        }
        std::cerr << "Error reading from client FD: " << clientFd << ": " << std::strerror(errno) << std::endl;
        removeClient(clientFd);
        return ;
    }
    if (bytesRead == 0)
    {
        removeClient(clientFd);
        return ;
    }
    buffer[bytesRead] = '\0';
    
    _clients[clientFd]->getBuffer().append(buffer, bytesRead);

    std::string& clientInput = _clients[clientFd]->getBuffer();
    size_t pos;

    while ((pos = clientInput.find('\n')) != std::string::npos)
    {
        std::string command = clientInput.substr(0, pos);

        if (!command.empty() && command[command.length() - 1] == '\r')
            command.erase(command.length() - 1);

        clientInput.erase(0, pos + 1);

        if (!command.empty())
            processCommand(clientFd, command);
        else
            std::cout << "Empty command ignored from FD " << clientFd << std::endl;
    }
}

void Server::processCommand(int clientFd, const std::string& command)
{
    std::string nick = _clients[clientFd]->getNickname().empty() ? "*" : _clients[clientFd]->getNickname();
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = command.find(' ');
    if (end == std::string::npos)
        tokens.push_back(command);
    else
        splitCommand(tokens, command, start, end);
    if (tokens.empty())
        return ;
    CommandType cmd = getCommandtype(tokens[0]);
    switch (cmd)
    {
        case CMD_PASS:
            handlePass(clientFd, tokens);
            break;
        case CMD_NICK:
            handleNick(clientFd, tokens);
            break;
        case CMD_USER:
            handleUser(clientFd, tokens);
            break;
        case CMD_PING:
            handlePing(clientFd, tokens);
            break;
        case CMD_PONG:
            handlePong(clientFd, tokens);
            break;
        case CMD_PRIVMSG:
            handlePrivmsg(clientFd, tokens);
            break;
        case CMD_JOIN:
            parseJoinCommand(clientFd, command);
            break;
        case CMD_TOPIC:
            parseTopicCommand(clientFd, command);
            break;
        case CMD_INVITE:
            inviteCommand(clientFd, tokens);
            break;
        case CMD_KICK:
            kickCommand(clientFd, tokens);
            break;
        case CMD_MODE:
            modeCommand(clientFd, tokens);
            break;
        case CMD_PART:
            handlePart(clientFd, command);
            break;
        case CMD_QUIT:
            handleQuit(clientFd, tokens);
            break;
        case CMD_UNKNOWN:
        default:
            std::cout << "New command: " << command << std::endl;
            sendReply(clientFd, macroToString(ERR_UNKNOWNCOMMAND) + " " + nick + " " + tokens[0] + " :Unknown command");
            break;

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
    std::cout << "Server shutdown complete." << std::endl;
}

// ----------------- SETTERS ----------------- 

void Server::setPort(int& port)
{
    _port = port;
}

// ----------------- GETTERS ----------------- 

int Server::getPort() const
{
    return (_port);
}

std::string Server::getPassword() const
{
    return (_password);
}

Client* Server::getClientByNickname(const std::string& nickname)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == nickname)
            return it->second;
    }
    return NULL;
}

const char* Server::PortOutOfBound::what() const throw()
{
    return ("Port must be between 1024 and 65535");
}

std::string Server::getClientPrefix(int fd) const // Taha fixed
{
    std::map<int, Client*>::const_iterator it = _clients.find(fd);
    if (it != _clients.end())
        return it->second->getPrefix();
    return "";
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
    ss << code << " " << _clients[fd]->getNickname() << " " << message<<std::endl;
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
    // Remove client from all channels first - Dina
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); )
    {
        Channel* channel = it->second;
        if (channel->isUser(clientFd))
        {
            channel->removeUser(clientFd);
            channel->removeOperator(clientFd); // Safe even if not operator

            // Optional: delete empty channel
            if (channel->getUserFds().empty())
            {
                delete channel;
                _channels.erase(it++);
                continue;
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

    // Delete the Client object and erase from map
    std::map<int, Client*>::iterator clientIt = _clients.find(clientFd);
    if (clientIt != _clients.end())
    {
        delete clientIt->second; // Free the memory - Dina
        _clients.erase(clientIt); // Remove from map - Dina
    }

    std::cout << "Client FD " << clientFd << RED << " disconnected!" << RESET << std::endl;
}
