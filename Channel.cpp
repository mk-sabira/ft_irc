#include "Channel.hpp"

Channel::Channel() {}
Channel& Channel::operator=(Channel& other) {}
Channel::Channel(Channel& other) {}
Channel::~Channel() {}



bool Channel::isOperator(int clientFd) const
{
    if (this->_operators.contains(clientFd)) // should use find
    return true;
    return false;
}

void Channel::addOperator(int clientFd)
{
    this->_operators.insert(clientFd);
}

void Channel::removeOperator(int clientFd)
{
    this->_operators.erase(clientFd);
}

std::string Channel::getName() const
{
    return this->_name;
}

std::set<int> Channel::getUserList() const
{
    std::set<int>   usersFds;

    std::map<int, Clients*>::iterator   it;
    for(it = _users.begin(); it != _users.end(); ++it)
        usersFds.insert(it->first);
    return usersFds;
}
