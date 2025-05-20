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
    // Remove client from all channels first - Dina
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); )
    {
        Channel* channel = it->second;
        if (channel->isUser(clientFd))
        {
            channel->removeUser(clientFd);
            channel->removeOperator(clientFd); // Safe even if not operator

            // Optional: delete empty channel
            if (channel->getUserFds().empty())
            {
                delete channel;
                _channels.erase(it++);
                continue;
            }
        }
        ++it;
    }
    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == clientFd)
        {
            _fds.erase(it);
            break;
        }
    }

    // Delete the Client object and erase from map
    std::map<int, Client*>::iterator clientIt = _clients.find(clientFd);
    if (clientIt != _clients.end())
    {
        delete clientIt->second; // Free the memory - Dina
        _clients.erase(clientIt); // Remove from map - Dina
    }

    std::cout << "Client FD " << clientFd << RED << " disconnected!" << RESET << std::endl;
}
