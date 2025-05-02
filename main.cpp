#include "main.hpp"
#include "Server.hpp"

int main(void)
{
    Server server;

    if (!server.setup(6667))
        return (1);
    if (!server.run())
        return (1);
    return (0);
}