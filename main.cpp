#include "main.hpp"
#include "Server.hpp"

int main(int arc, char** arv)
{
    if ( arc != 3)
    {
        std::cout << "Program runs with: ./ircserv <port> <password>" << std::endl;
        return (1);
    }
    else
    {
        try
        {
            Server server(arv[1], arv[2]);
            server.serverSetup();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }
    return (0);
}