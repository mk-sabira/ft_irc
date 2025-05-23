/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 07:53:52 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/23 09:43:19 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"


void signalHandler(int signum)
{
    (void)signum;
    Server::keepRunning = 0;
    // signal(SIGINT, SIG_IGN);
    signal(SIGINT, SIG_DFL);
    const char msg[] = "\nReceived SIGINT, shutting down server...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return (1);
    }
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    if (sigaction(SIGINT, &sa, 0) == -1)
    {
        std::cerr << "Failed to set SIGINT handler: " << strerror(errno) << std::endl;
        return 1;
    }
    try
    {
        Server server(argv[1], argv[2]);

        if (!server.serverSetup())
        {
            std::cerr << "Server setup failed." << std::endl;
            return (1);
        }

        server.runServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    return (0);
}
