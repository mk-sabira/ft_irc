/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:10:44 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/06 11:00:51 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <sstream>
#include <exception>
#include <fcntl.h>
#include <vector>
#include <map>




#include <unistd.h> 
#include <netinet/in.h>   // For sockaddr_in structure, which is used for specifying socket addresses
#include <poll.h>


class Server
{
private:
    int _serverFd;
    int _port;
    std::string _password;
    struct sockaddr_in _serverAdd;
    std::vector<struct pollfd> _fds;

    
    
    
    class Client
    {
        private:
            int _fd;
            std::string _buffer;
            std::string _nickname;
            std::string _username;
            bool _authenticated;
        
        public:
            // Getter and Setter for _buffer
            std::string& getBuffer() { return _buffer; }
            void setBuffer(const std::string& buffer) { _buffer = buffer; }
        
            // Other getters and setters (e.g., for _fd and _authenticated)
            int getFd() const { return _fd; }
            void setFd(int fd) { _fd = fd; }
        
            bool isAuthenticated() const { return _authenticated; }
            void setAuthenticated(bool authenticated) { _authenticated = authenticated; }
    };
    

    std::map<int, Client> _clients;
    void acceptNewClient();
    void recieveData(int clientFd);
    void processCommand(int clientFd, const std::string& cmd);
public:
    Server(const std::string& port, const std::string& password);
    ~Server();

    //Methods
    bool serverSetup();
    bool runServer();

    //setters
    void setPort(int& port);
    void setPassword(std::string& password);

    //getters
    int getPort() const;
    std::string getPassword() const;
    
    //exceptions
    class PortOutOfBound: public std::exception
    {
        public:
        const char* what() const throw();
    };
};

#endif