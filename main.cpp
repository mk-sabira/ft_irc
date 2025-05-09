/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 07:53:52 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/09 07:53:56 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"

int main(int argc, char **argv)
{
    try
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
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }

    return (0);
}
