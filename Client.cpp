
#include "Client.hpp"

Client::Client(){}

Client::~Client(){}

Client::Client(const Client &copy)
{
    (void) copy;
    //all the atrubutes must be copied
}

Client &Client::operator=(const Client &obj)
{
    (void) obj;
    // TODO: insert return statement here
    // must be reassigned
    return (*this);
}

//getters
std::string& Client::getBuffer()
{ 
    return _buffer; 
}

int Client::getFd() const
{
    return (_fd);
}

bool Client::isAuthenticated() const
{
    return (_authenticated);
}

std::string &Client::getNickname()
{
    return (_nickname);
}

std::string &Client::getUsername()
{
    return (_username);
}

std::string &Client::getRealname()
{
    return (_realname);
}

bool Client::isRegistered() const
{
    return (_registered);
}

//setters
void Client::setBuffer(const std::string& buffer)
{
    _buffer = buffer;
}

void Client::setFd(int fd)
{
    _fd = fd;
}

void Client::setAuthenticated(bool authenticated)
{
    _authenticated = authenticated;
}

void Client::setNickname(const std::string &nickname)
{
    _nickname  = nickname;
}

void Client::setUsername(const std::string &username)
{
    _username = username;
}

void Client::setRealname(const std::string &realname)
{
    _realname = realname;
}

void Client::setRegistered(bool registered)
{
    _registered = registered;
}