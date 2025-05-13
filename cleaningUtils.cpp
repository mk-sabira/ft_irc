/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleaningUtils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 10:09:39 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/12 11:12:40 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::removeClient(int clientFd)
{
    close(clientFd);
    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == clientFd)
        {
            _fds.erase(it);
            break;
        }
    }
    _clients.erase(clientFd);
    std::cout << "Client FD " << clientFd << RED << " disconnected!" << RESET << std::endl;
}

