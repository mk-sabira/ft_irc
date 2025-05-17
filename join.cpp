#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

void Server::parseJoinCommand(int userFd, const std::string& command)
{
    std::vector<std::string> tokens;
    size_t                   start = 0;
    size_t                   end = command.find(' ');

    while (end != std::string::npos && command[start] == ' ')
    {
        ++start;
        end = command.find(' ', start);
    }

    if (end == std::string::npos)
        return;

    tokens.push_back(command.substr(start, end - start));
    start = end + 1;

    while (start < command.length() && command[start] == ' ')
        ++start;

    end = command.find(' ', start);
    std::string channels = command.substr(start, end - start);

    std::string keys;
    if (end != std::string::npos)
    {
        start = end + 1;
        while (start < command.length() && command[start] == ' ')
            ++start;
        if (start < command.length())
            keys = command.substr(start);
    }

    // Split channels
    std::vector<std::string> channelList;
    std::string::size_type chanStart = 0;
    std::string::size_type chanEnd;
    while ((chanEnd = channels.find(',', chanStart)) != std::string::npos)
    {
        channelList.push_back(channels.substr(chanStart, chanEnd - chanStart));
        chanStart = chanEnd + 1;
    }
    if (chanStart < channels.length())
        channelList.push_back(channels.substr(chanStart));

    // Split keys
    std::vector<std::string> keyList;
    std::string::size_type keyStart = 0;
    std::string::size_type keyEnd;
    while ((keyEnd = keys.find(',', keyStart)) != std::string::npos)
    {
        keyList.push_back(keys.substr(keyStart, keyEnd - keyStart));
        keyStart = keyEnd + 1;
    }
    if (keyStart < keys.length())
        keyList.push_back(keys.substr(keyStart));

    // Join each channel with corresponding key
    for (std::size_t i = 0; i < channelList.size(); ++i)
    {
        std::string key = (i < keyList.size()) ? keyList[i] : "";
        joinCommand(userFd, channelList[i], key);
    }
}
