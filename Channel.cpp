#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"

Channel::Channel() {}
Channel::Channel(const std::string& name) : _name(name) {}
Channel& Channel::operator=(Channel& other) {}
Channel::Channel(Channel& other) {}
Channel::~Channel() {}



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

std::set<int> Channel::getUserFds() const
{
    std::set<int>   usersFds;

    std::map<int, Clients*>::iterator   it;
    for(it = _users.begin(); it != _users.end(); ++it)
        usersFds.insert(it->first);
    return usersFds;
}

std::vector<std::string>    Channel::getNicknamesWithPrefixes() const
{
    std::vector<std::string> listOfUsers;
    for (std::map<int, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it) {
        Client* client = it->second;
        std::string prefix;

        if (isOperator(client->getFd()))
            prefix = "@";
        listOfUsers.push_back(prefix + client->getNickname());
    }
    return listOfUsers;
}

//------------------- SETTERS ----------------------------

void Channel::setKey(const std::string& key)
{
    this->_key = key;
}

void Channel::setTopic(const std::string& topic)
{
    this->_topic = topic;
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

// void Channel::broadcastToAll(const std::string& message, Server* server)
// {
//     for (std::map<int, Client*>::iterator it = _users.begin(); it != _users.end(); ++it)
//     {
//         int clientFd = it->first;
//         server->sendMessage(clientFd, message);
//     }
// }
//-------------------- JOIN --------------------------
bool Channel::canJoin(int clientFd, const std::string& key)
{
    if (this->hasKey() && this->getKey() != key)
        return false;
    return true;

}

void Channel::addUser(int clientFd)
{
    Client* client = getClientByFd(clientFd); // client functions
    if (!client)
        return; // Handle null pointer

    this->_users.insert(std::pair<int, Client*>(clientFd, client));
    _invited.erase(clientFd);
}

//------------ TOPIC -----------------

bool Channel::isTopicRestricted() const {
    return this->_topicRestricted;
}

void Channel::setTopic(const std::string& topic) {
    this->_topic = topic;
}

void Channel::clearTopic() {
    this->_topic.clear();
}

std::string Channel::getTopic() const {
    return this->_topic;
}

std::string Channel::getClientPrefix(int fd) const
{
    if (isOperator(fd))
        return ("@");
    return ("");
}



void Channel::addOperator(int clientFd)
{
    this->_operators.insert(clientFd);
}

void Channel::addInvite(int clientFd)
{
    this->_invited.insert(clientFd);
}


void Channel::removeUser(int clientFd)
{
    this->_users.erase(clientFd);
}