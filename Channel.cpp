#include "Channel.hpp"

Channel::Channel() {}
Channel& Channel::operator=(Channel& other) {}
Channel::Channel(Channel& other) {}
Channel::~Channel() {}



// void Channel::addOperator(int clientFd)
// {
//     this->_operators.insert(clientFd);
// }

void Channel::removeOperator(int clientFd)
{
    this->_operators.erase(clientFd);
}
//----------------- Getters -----------------------------

std::string Channel::getName() const
{
    return this->_name;
}

std::string Channel::getTopic() const
{
    return this->_topic;
}

std::string Channel::getKey() const
{
    return this->_key;
}

std::set<int> Channel::getUserList() const
{
    std::set<int>   usersFds;

    std::map<int, Clients*>::iterator   it;
    for(it = _users.begin(); it != _users.end(); ++it)
        usersFds.insert(it->first);
    return usersFds;
}

        //---------------helper functions---------------

 bool Channel::isUser(int clientFd) const
{
    if (this->_users.find(clientFd) != this->_users.end())
        return true;
    return false;
}
bool Channel::isOperator(int clientFd) const
{
    if (this->_operators.find(clientFd) != this->_operators.end())
        return true;
    return false;
}

bool Channel::isInviteOnly() const
{
    if (this->inviteOnly)
        return true;
    return false;
}

bool Channel::isInvited(int clientFd) const
{
    if (this->_invited.find(clientFd) != this->_invited.end())
        return true;
    return false;
}

bool Channel::hasKey() const
{
    if (this->_key.empty())
        return false;
    return true;
}

bool Channel::isFull() const
{
    if (this->_users.size() != userLimit)
        return false;
    return true;
}

//-------------------- JOIN --------------------------
void Channel::addUser(int clientFd)
{
    Client* client = getClientByFd(clientFd); // client functions
    if (!client)
        return; // Handle null pointer

    this->_users.insert(std::pair<int, Client*>(clientFd, client));
    _invited.erase(clientFd);
}

bool Channel::canJoin(int clientFd, const std::string& key)
{
    if (this->hasKey() && this->getKey() != key)
        return false;
    return true;

}



void Channel::addOperator(int clientFd)
{
    this->_operators.insert(clientFd);
}