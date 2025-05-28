/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:10:44 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/28 10:21:11 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <cstdlib> // for atoi
#include <sys/socket.h>
#include <sstream>
#include <exception>
#include <fcntl.h>
#include <vector>
#include <map>
#include <sstream>  // Needed for std::istringstream


#include <csignal>
#include <cstring>

#include <unistd.h> 
#include <netinet/in.h>   // For sockaddr_in structure, which is used for specifying socket addresses
#include <poll.h>

#include "colors.hpp"
#include "Client.hpp"
#include "Channel.hpp" // Dina Channel


#define ERR_NEEDMOREPARAMS      461 // PASS, USER, PING, PONG, PRIVMSG
#define ERR_ALREADYREGISTERED   462 // PASS, USER
#define ERR_PASSWDMISMATCH      464 // PASS
#define ERR_NONICKNAMEGIVEN     431 // NICK
#define ERR_ERRONEUSNICKNAME    432 // NICK
#define ERR_NICKNAMEINUSE       433 // NICK
#define ERR_NICKCOLLISION       436 // NICK
#define ERR_NOORIGIN            409 // PING
#define ERR_NOOPERHOST          491 // PONG
#define RPL_YOUREOPER           381 // PONG
#define ERR_NOSUCHNICK          401 // PRIVMSG
#define ERR_NOSUCHSERVER        402 // PRIVMSG
#define ERR_CANNOTSENDTOCHAN    404 // PRIVMSG
#define ERR_TOOMANYTARGETS      407 // PRIVMSG
#define ERR_NORECIPIENT         411 // PRIVMSG
#define ERR_NOTEXTTOSEND        412 // PRIVMSG
#define ERR_NOTOPLEVEL          413 // PRIVMSG
#define ERR_WILDTOPLEVEL        414 // PRIVMSG
#define RPL_AWAY                301 // PRIVMSG
#define RPL_WELCOME             001 // USER, NICK
#define RPL_YOURHOST            002 // USER, NICK
#define RPL_CREATED             003 // USER, NICK
#define RPL_MYINFO              004 // USER, NICK
#define ERR_UNKNOWNCOMMAND 		421 // UNKNOWN


enum CommandType
{
	CMD_PASS,
	CMD_NICK,
	CMD_USER,
	CMD_PING,
	CMD_PONG,
	CMD_PRIVMSG,
	CMD_JOIN,
	CMD_TOPIC,
	CMD_INVITE,
	CMD_KICK,
	CMD_MODE,
	CMD_PART,
	CMD_QUIT,
	CMD_UNKNOWN,
};

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
	    std::map<std::string, Channel*> _channels; // <name, obj> // Dina Channel // Taha compilation error
	    std::map<std::string, int> _nickToFd; // <nickname, fd> // Dina Channel // Taha compilation error
	    
		void welcomeMessage();
	    void acceptNewClient();
	    void receiveClientData(int clientFd);
	    bool processCommand(int clientFd, const std::string& cmd);
	    void removeClient(int clientFd);
	    void splitCommand(std::vector<std::string>& tokens, const std::string& command, std::string::size_type start, std::string::size_type end );
	
	    void handlePass(int clientFd, const std::vector<std::string>& tokens);
	    
	    void handleNick(int clientFd, const std::vector<std::string>& tokens);
	    bool validateNick(const std::string& nick);
	    
	    void handleUser(int clientFd, const std::vector<std::string>& tokens);
	    bool validateUser(int clientFd, const std::vector<std::string>& tokens, std::string& errorMsg);
		
	    void handlePrivmsg(int clientFd, const std::vector<std::string>& tokens);
		bool validatePrivmsg(int senderFd, const std::vector<std::string>& tokens, std::string& errorMsg);
		std::string buildPrivmsg(const std::vector<std::string>& tokens);
	    void sendToChannelTarget(int senderFd, const std::string& target, const std::string& message);
	    void sendToClientTarget(int senderFd, const std::string& target, const std::string& message);
		
	    void handlePing(int clientFd, const std::vector<std::string>& tokens);
	    void handlePong(int clientFd, const std::vector<std::string>& tokens);
		
	    void handlePart(int clientFd, const std::string& command);
	    void handleQuit(int clientFd, const std::vector<std::string>& tokens);
		std::string macroToString(int macro);
		void shutdownMessage();
public:
		static volatile sig_atomic_t keepRunning;
	    Server(const std::string& port, const std::string& password);
	    ~Server();
		
	    //Methods
	    bool serverSetup();
	    bool runServer();
		void shutdown();
		
	    //setters
	    void setPort(int& port);
		
	    //getters
	    int getPort() const;
	    std::string getPassword() const;
		CommandType getCommandtype (const std::string& command);
	    
		//------------------------ // Dina Channel
	    Client* getClientByNickname(const std::string&);
	    // std::string getClientNickname(int fd) const; // taha compilation error__.
	    
	    
	    //exceptions
	    class PortOutOfBound: public std::exception
	    {
			public:
	        const char* what() const throw();
	    };
	    
	    //----- helper functions----------- // Dina Channel
		void sendReply(int clientFd, const std::string& message);
		void boolSendReply(int clientFd, const std::string& message, bool useServerPrefix); // taha fixing limechat
		void sendRaw(int clientFd, const std::string& rawMessage); // taha for end command
	    void    sendError(int userFd, int errorCode, const std::string& message);
	    void    sendToClient(int fd, int code, const std::string& message);
		void    boolSendToClient(int fd, int code, const std::string& message);
	    //-------- CHANNEL ---------------------- // Dina channel
	    void broadcastToAll(const Channel& channel, const std::string& msg, int excludeFd); // Taha compilation error
		
		void 	parseJoinCommand(int userFd, const std::string& command);
	    void    joinCommand(int userFd, std::string channelName, std::string key);
		void	parseTopicCommand( int userFd, const std::string& command);
	    void    topicCommand(int userFd, std::string channelName, std::string topic, bool colon);
	    void    kickCommand(int senderFd, const std::vector<std::string>& tokens);
	    void    inviteCommand(int senderFd, const std::vector<std::string>& tokens);
		void	modeCommand(int userFd, const std::vector<std::string>& tokens);
		void	partUser(int clientFd, const std::string& channelName, const std::string& comment);

	    
	    //------ CHANNEL Taha --------//
	    
	    std::string getClientPrefix(int fd) const;
};

bool		stringToInt(const std::string& str, int& result);
std::string vecToStr(std::vector<std::string> vec);

#endif