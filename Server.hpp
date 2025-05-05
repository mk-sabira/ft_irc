/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:10:44 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/05 12:16:07 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/socket.h>
#include <sstream>
#include <exception>




#include <vector>
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

public:
    Server(const std::string& port, const std::string& password);
    ~Server();

    //Methods
    bool serverSetup();
    bool runServer();


    //exceptions
    class PortOutOfBound: public std::exception
    {
        public:
        const char* what() const throw();
    };
};