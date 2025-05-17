/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:10:44 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/09 12:11:13 by bmakhama         ###   ########.fr       */
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

// #include "colors.hpp"
#include "Client.hpp"
#include "Channel.hpp"


class Server
{
private:
    int _serverFd;
    int _port;
    std::string _password;
    struct sockaddr_in _serverAdd;
    std::vector<struct pollfd> _fds;
    std::map<int, Client*> _clients;
    std::string _serverName;
    std::map<std::string, Channel*> _channels; // <name, obj>
    std::map<std::string, int> _nickToFd; // <nickname, fd>
    
    void acceptNewClient();
    void recieveClientData(int clientFd);
    void processCommand(int clientFd, const std::string& cmd);
    void removeClient(int clientFd);
    void splitCommand(std::vector<std::string>& tokens, const std::string& command, std::string::size_type start, std::string::size_type end );

    void handlePass(int clientFd, const std::vector<std::string>& tokens);
    void handleNick(int clientFd, const std::vector<std::string>& tokens);
    void handleUser(int clientFd, const std::vector<std::string>& tokens);
    void sendReply(int clientFd, const std::string& message);


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
    //------------------------
    Client* getClientByNickname(const std::string&);
    const std::string&  getClientNickname(int fd); 
    
    //exceptions
    class PortOutOfBound: public std::exception
    {
        public:
        const char* what() const throw();
    };

    //----- helper functions-----------
    void    sendError(int userFd, int errorCode, const std::string& message);
    void    sendMessage(int fd, const std::string& message);
    void    sendToClient(int fd, int code, const std::string& message);

    //-------- CHANNEL ----------------------
    void broadcastToAll(const Channel& channel, const std::string& msg, int excludeFd);

    void    parseJoinCommand(int userFd, const std::string& command);

    void    joinCommand(int userFd, std::string channelName, std::string key);
    void    topicCommand(int userFd, const std::vector<std::string>& tokens);
    void    kickCommand(int senderFd, const std::vector<std::string>& tokens);
    void    inviteCommand(int senderFd, const std::vector<std::string>& tokens);

};

#endif