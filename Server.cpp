/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:25:46 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/11 14:31:53 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::~Server()
{
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

// Create a socket using the system call for IPv4 and TCP.
// Set up a sockaddr_in structure to define:
// IP address: accept any (INADDR_ANY)
// Port number: use the given port (e.g. 6667)
// Address family: IPv4
// Bind the socket to the address and port so the OS knows you want to listen there.
// Start listening on the socket

bool Server::serverSetup()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd == -1)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return (false);
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

    Client client;
    client.setFd(clientFd);
    client.setAuthenticated(false);
    _clients[clientFd] = client;
    std::cout << "New client connected: FD = " << clientFd << std::endl;
}

void Server::recieveClientData(int clientFd)
{
    size_t bytesRead;
    char buffer[1024];

    bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return ;
        std::cerr << "Error reading from client FD: " << clientFd << ": " << strerror(errno) << std::endl;
        removeClient(clientFd);
        return ;
    }
    if (bytesRead == 0)
    {
        removeClient(clientFd);
        return ;
    }
    buffer[bytesRead] = '\0';
    
    _clients[clientFd].getBuffer().append(buffer, bytesRead);
    std::string& clientInput = _clients[clientFd].getBuffer();
    size_t pos;

    while ((pos = clientInput.find("\r\n")) != std::string::npos)
    {
        
        std::string command = clientInput.substr(0, pos); //// Extract command (from start to \r\n)
        clientInput.erase(0, pos + 2); //// Remove command and \r\n from buffer
        if (!command.empty())
        {
            std::cout << "Received command from FD " << clientFd << ": " << command << std::endl;
            processCommand(clientFd, command); 
        }
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
    if (tokens[0] == "PASS")
    {
        std::cout << "password: " << RED << tokens[1] << RESET << std::endl;
        handlePass(clientFd, tokens);
    }
    else if( tokens[0] == "NICK")
    {
        std::cout << "NICK: " << BLUE << tokens[1] << RESET << std::endl;
        handleNick(clientFd, tokens);
    }
    else if (tokens[0] == "USER")
    {
        std::cout << "USER name: " << GREEN << tokens[1] << RESET << std::endl;
        std::cout << "Real name: " << GREEN << tokens[4] << RESET << std::endl;
        handleUser(clientFd, tokens);
    }
    else if (tokens[0] == "PRIVMSG")
    {
        std::cout << "New input: " << tokens[0] << " Second: " << tokens[1] << std::endl;
        handlePrivmsg(clientFd, tokens);
    }
    else if (tokens[0] == "JOIN")
    {
        // handleJoin(clientFd, tokens);
    }
    else
        sendReply(clientFd, "421 " + tokens[0] + " :Unknown command");
        
    
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