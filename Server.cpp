/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrhelmy <mrhelmy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:25:46 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/17 23:57:24 by mrhelmy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cerrno>
#include <arpa/inet.h>

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
    if (_port < 1024 || _port > 65535)
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

    //new line -> Enable SO_REUSEADDR
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
    while (true)
    {
        int pollCount = poll(&_fds[0], _fds.size(), -1);
        if (pollCount == -1)
        {
            std::cerr << "poll() failed" << std::endl;
            return (false);
        }

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].fd == _serverFd && _fds[i].revents & POLLIN)
                acceptNewClient();
            else if (_fds[i].fd != _serverFd && _fds[i].revents & POLLIN)
                recieveClientData(_fds[i].fd);
            _fds[i].revents = 0; //new line
        }
    }
    std::cout << "Server stopped running" << std::endl;
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
    std::cout << CYAN << "New client connected: FD = " << clientFd << RESET << std::endl;
}

void Server::recieveClientData(int clientFd)
{
    int bytesRead;
    char buffer[1024];

    // std::cout << "Checking FD " << clientFd << " for data" << std::endl;
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
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = command.find(' ');
    if (end == std::string::npos)
        tokens.push_back(command);
    else
        splitCommand(tokens, command, start, end);
    if (tokens.empty())
        return ;
    if (tokens[0] == "PASS" || tokens[0] == "pass")
    {
        // std::cout << "password: " << RED << tokens[1] << RESET << std::endl;
        handlePass(clientFd, tokens);
    }
    else if( tokens[0] == "NICK" || tokens[0] == "nick")
    {
        // std::cout << "NICK: " << BLUE << tokens[1] << RESET << std::endl;
        handleNick(clientFd, tokens);
    }
    else if (tokens[0] == "USER" || tokens[0] == "user")
    {
        std::cout << "USER name: " << GREEN << tokens[1] << RESET << std::endl;
        std::cout << "Real name: " << GREEN << tokens[4] << RESET << std::endl;
        handleUser(clientFd, tokens);
    }
    else if (tokens[0] == "PING" || tokens[0] == "ping")
    {
        std::cout << "PING cout: " << YELLOW << command << RESET << std::endl;
        handlePing(clientFd, tokens);
    }
    else if (tokens[0] == "PRIVMSG")
    {
        handlePrivmsg(clientFd, tokens);
    }
    else if (tokens[0] == "JOIN")  // compilation Error Taha
	{
	    // if (tokens.size() < 2)
	    // {
	    //     sendReply(clientFd, "461 JOIN :Not enough parameters");
	    //     return;
	    // }
	    // std::string key = (tokens.size() > 2) ? tokens[2] : "";
	    parseJoinCommand(clientFd, command);

	}
	else if (tokens[0] == "TOPIC") // compilation Error Taha
	{
	    parseTopicCommand(clientFd, command);
	}
	else if (tokens[0] == "INVITE") // compilation Error Taha
	{
	    inviteCommand(clientFd, tokens);
	}
	else if (tokens[0] == "KICK")
	{
	    kickCommand(clientFd, tokens);
	}
	else if (tokens[0] == "MODE") // compilation Error Taha
	{
	    modeCommand(clientFd, tokens);
	}
    else
    {
        std::cout << "Unknown cout: " << YELLOW << tokens[0] << RESET << std::endl;
        sendReply(clientFd, "421 " + tokens[0] + " :Unknown command");
    }
    
    
}



//setters
void Server::setPort(int& port)
{
    _port = port;
}

void Server::setPassword(std::string& password)
{
    _password = password;
}


//getters
int Server::getPort() const
{
    return (_port);
}

std::string Server::getPassword() const
{
    return (_password);
}

const char* Server::PortOutOfBound::what() const throw()
{
    return ("Port must be between 1024 and 65535");
}


///------ Taha for the channel-------
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
    ss << code << " " << _clients[fd]->getNickname() << " " << message << "\r\n";
    sendReply(fd, ss.str());
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