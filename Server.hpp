/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/02 10:10:44 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/02 12:33:22 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <unistd.h> 
#include <netinet/in.h>   // For sockaddr_in structure, which is used for specifying socket addresses
#include <poll.h>


class Server
{
private:
    int _serverFd;
    struct sockaddr_in _serverAdd;

public:
    Server();
    ~Server();

    bool setup(int port);
    bool run();
};