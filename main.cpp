/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 07:53:52 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/22 09:45:14 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "colors.hpp"

// static volatile sig_atomic_t keepRunning = 1;

// void SignalHandler(int signum)
// {
//     (void) signum;
//     keepRunning = 0;
//     std::cout << RED << "Received SIGINT, shutting down server..." << std::endl;
// }

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return (1);
    }

    Server server(argv[1], argv[2]);

    if (!server.serverSetup())
    {
        std::cerr << "Server setup failed." << std::endl;
        return (1);
    }

    server.runServer();

    return (0);
}
