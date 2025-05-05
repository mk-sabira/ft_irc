/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:25:46 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/05 12:28:32 by bmakhama         ###   ########.fr       */
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
    // std::cout << _serverFd << std::endl;
    return (true);
}


// The server waits for new client connections
// Accepts them
// Monitors existing clients for new messages or disconnections
// Handles input/output events

bool Server::runServer()
{
    
    return (false);
}

// // Main loop to handle connections and messages
// bool Server::run() {
//     // Array of pollfd structs to monitor multiple file descriptors (server + clients)
//     struct pollfd fds[100];  // You can increase this number as needed

//     int nfds = 1; // Number of file descriptors currently tracked
//     fds[0].fd = _serverFd;   // First FD is the server socket (used to accept clients)
//     fds[0].events = POLLIN;   // Watch for readable events (incoming connections)

//     // Infinite loop: server runs forever
//     while (true) {
//         // Wait until any socket has data or connection using poll()
//         int ret = poll(fds, nfds, -1);  // -1 means wait indefinitely

//         if (ret < 0) {
//             std::cerr << "Poll error\n";  // If poll fails, print error
//             return false;
//         }

//         // Check if the server socket has an incoming connection
//         if (fds[0].revents & POLLIN) {
//             // Accept the new client connection
//             int client_fd = accept(_serverFd, NULL, NULL);
//             if (client_fd != -1)
//             {
//                 // Add the new client socket to the poll array
//                 fds[nfds].fd = client_fd;
//                 fds[nfds].events = POLLIN;  // We want to read from this client
//                 nfds++; // Increase number of tracked sockets

//                 std::cout << "New client connected: fd = " << client_fd << std::endl;

//                 // Send IRC welcome message to the client
//                 std::string nickname = "guest"; // You can update this later when parsing NICK command
//                 std::string welcome = ":localhost 001 " + nickname + " :Welcome to the IRC server\r\n";

//                 // Send message to client
//                 send(client_fd, welcome.c_str(), welcome.length(), 0);
//             }
//         }

//         // Loop through all connected clients (skip index 0 which is server)
//         for (int i = 1; i < nfds; ++i) {
//             // If this client socket has data to read
//             if (fds[i].revents & POLLIN) {
//                 char buffer[512];  // Temporary buffer to store the message
//                 int bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);  // Receive data

//                 if (bytes <= 0) {
//                     // If recv fails or client disconnects
//                     std::cout << "Client disconnected: fd = " << fds[i].fd << std::endl;
//                     close(fds[i].fd); // Close that client's socket

//                     // Remove this client from the poll array by replacing it with the last one
//                     fds[i] = fds[nfds - 1];
//                     nfds--;  // Decrease the number of active sockets
//                     i--;     // Stay on the same index after replacing
//                 } else {
//                     // Null-terminate the received message and print it
//                     buffer[bytes] = '\0';
//                     std::cout << "Received from client " << fds[i].fd << ": " << buffer;
//                 }
//             }
//         }
//     }

//     return true;
// }





const char* Server::PortOutOfBound::what() const throw()
{
    return ("Port must be between 1024 and 65535");
}