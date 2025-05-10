#include "Channel.hpp"

void sendError(int userFd, int errorCode, const std::string& target, const std::string& message = "")
{
    std::string nickname = getClientNickname(userFd); // safely fallback if not registered
    std::stringstream ss;

    ss << ":" << SERVER_NAME << " " << errorCode << " "
       << (nickname.empty() ? "*" : nickname) << " "
       << target << " :" << message;

    sendToClient(userFd, ss.str());
}
