/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:25:46 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/07 18:57:05 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::~Server()
{
    if (_serverFd != -1)
        close(_serverFd);
}


Server::Server(const std::string &port, const std::string &password):_serverFd(-1)
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
    
    std::cout << _serverFd << std::endl;
    return (true);
}


// The server waits for new client connections
// Accepts them
// Monitors existing clients for new messages or disconnections
// Handles input/output events

// Waits for activity using select() (or poll() / epoll()).
// Accepts new client connections when a client tries to connect.
// Reads incoming messages from already-connected clients.
// Handles disconnections if a client closes the connection or crashes.
// Dispatches data to the right handler (e.g., IRC commands like NICK, USER).
bool Server::runServer()
{
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
            // If it's the server socket and it's ready for reading (incoming connection)
            if (_fds[i].fd == _serverFd && _fds[i].revents && POLLIN)
                acceptNewClient();
            // If it's a client socket and it's ready for reading (incoming data)
            else if (_fds[i].fd != _serverFd && _fds[i].revents && POLLIN)
                recieveClientData(_fds[i].fd);
        }
    }
    
    return (false);
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


//The receiveData method is responsible for:
// Reading data from a client’s TCP socket (identified by clientFd).
// Buffering partial or complete IRC commands (text ending with \r\n).
// Processing complete commands by passing them to processCommand.
// Handling errors, disconnections, and non-blocking I/O.


void Server::recieveClientData(int clientFd)
{
    size_t bytesRead;
    char buffer[1024];

    bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead < 0)
    if (bytesRead == 0)
        //todo;
        //todo
    buffer[bytesRead] = '\0';
        
}

// void Server::recieveClientData(int clientFd)
// {
//     char buffer[512];
//     size_t bytesRead;
    
//     bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
//     if (bytesRead < 0)
//     {
//         if (errno == EAGAIN || errno == EWOULDBLOCK) {
//             return;
//         }
//         std::cerr << "Error reading from client FD " << clientFd << ": " << strerror(errno) << std::endl;
//         close(clientFd);
//         _clients.erase(clientFd);
//         for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) 
//         {
//             if (it->fd == clientFd)
//             {
//                 _fds.erase(it);
//                 break;
//             }
//         }
//         std::cout << "Client FD " << clientFd << " disconnected" << std::endl;
//         return;
//     }
//     if (bytesRead == 0)
//     {
//         close(clientFd);
//         _clients.erase(clientFd);
//         for (std::vector<struct pollfd>::iterator it  = _fds.begin(); it != _fds.end(); it++)
//         {
//             if (it->fd == clientFd)
//             {
//                 _fds.erase(it);
//                 break;
//             }
//         }
//         std::cout << "Client FD " << clientFd << " disconnected" << std::endl;
//         return ;
//     }
//     buffer[bytesRead] = '\0';

    // _clients[clientFd].getBuffer() += buffer;

    // std::string::size_type pos;
    // while ((pos = _clients[clientFd].getBuffer().find("\r\n")) != std::string::npos)
    // {
    //     std::string command = _clients[clientFd].getBuffer().substr(0, pos);
    //     _clients[clientFd].getBuffer().erase(0, pos + 2); // Remove processed command
    //     if (!command.empty())
    //     {
    //         processCommand(clientFd, command);
    //     }
    // }
// }

void Server::processCommand(int clientFd, const std::string& command)
{
    std::cout << "Received from FD: " << clientFd << "; command: " << command << std::endl;

    // Parse command into tokens
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = command.find(' ');
    while (end != std::string::npos) {
        tokens.push_back(command.substr(start, end - start));
        start = end + 1;
        end = command.find(' ', start);
    }
    tokens.push_back(command.substr(start));

    if (tokens.empty()) {
        return;
    }

    std::string cmd = tokens[0];
    // Convert to uppercase for case-insensitive comparison
    for (std::string::size_type i = 0; i < cmd.length(); ++i) {
        cmd[i] = std::toupper(cmd[i]);
    }

    // Handle PASS command
    if (cmd == "PASS") {
        if (tokens.size() < 2) {
            std::string reply = ":server 461 PASS :Not enough parameters\r\n";
            send(clientFd, reply.c_str(), reply.length(), 0);
            return;
        }
        if (_clients[clientFd].isAuthenticated())
        {
            std::string reply = ":server 462 :You may not reregister\r\n";
            send(clientFd, reply.c_str(), reply.length(), 0);
            return;
        }
        if (tokens[1] == _password) {
            _clients[clientFd].setAuthenticated(true);
            std::cout << "Client FD " << clientFd << " authenticated" << std::endl;
        } else {
            std::string reply = ":server 464 :Password incorrect\r\n";
            send(clientFd, reply.c_str(), reply.length(), 0);
            close(clientFd);
            _clients.erase(clientFd);
            for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
                if (it->fd == clientFd) {
                    _fds.erase(it);
                    break;
                }
            }
            std::cout << "Client FD " << clientFd << " disconnected (wrong password)" << std::endl;
        }
    } else {
        // For testing, reject non-PASS commands
        std::string reply = ":server 421 " + cmd + " :Unknown command\r\n";
        send(clientFd, reply.c_str(), reply.length(), 0);
    }
    std::cout << "Token: " << tokens.size() << std::endl; 

    // for (size_t i = 0; i < tokens.size(); i++)
    // {
    //     std::cout << "Token: " << tokens[i] << std::endl; 
        
    // }
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