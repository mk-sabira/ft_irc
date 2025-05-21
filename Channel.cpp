#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"


//----------------- Constructors + Destructor --------------------
Channel::Channel()
    : _name(""),
      _topic(""),
      _key(""),
      _userLimit(-1),
      _inviteOnly(false),
      _topicRestricted(false)
{}
Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _key(""),
      _userLimit(-1),
      _inviteOnly(false),
      _topicRestricted(false)
{}
Channel& Channel::operator=(Channel& other) {
    (void)other;
    return *this;}
Channel::Channel(Channel& other) { (void) other;}
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
    std::set<int> usersFds;

    std::map<int, Client*>::const_iterator it;
    for (it = _users.begin(); it != _users.end(); ++it)
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

void Channel::setName(const std::string& channelName)
{
        this->_name = channelName;

}

void Channel::setInviteFlag(const char   sign)
{
    if (sign == '+')
        this->_inviteOnly = true;
    else
        this->_inviteOnly = false;
}

void Channel::setRestrictions(const char   sign)
{
    if (sign == '+')
        this->_topicRestricted = true;
    else
        this->_topicRestricted = false;
}

void Channel::setKeyMode(const char   sign, const std::string& key)
{
    if (sign == '+')
        this->_key = key;
    else
        this->_key = "";
    }
    
void Channel::setOperatorMode(const char   sign, int userFd)
{
    if (sign == '+')
        this->addOperator(userFd);
    else
        this->removeOperator(userFd);
}

void Channel::setUserLimit(const char   sign, int limit)
{
    if (sign == '+' && limit < 1)
    {
        //error
        return ;
    }
    this->_userLimit = limit;
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
    // if (this->_inviteOnly)
    //     return true;
    return _inviteOnly;
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
    if (_userLimit == -1)
        return false;
    if (this->_users.size() !=(unsigned int) _userLimit)
        return false;
    return true;
}

void Channel::broadcastToAll(const std::string& message, Server* server)
{
    for (std::map<int, Client*>::iterator it = _users.begin(); it != _users.end(); ++it)
    {
        int clientFd = it->first;
        server->sendReply(clientFd, message);
    }
}

void Channel::boolBroadCastToAll(const std::string& message, Server* server, bool useServerPrefix)
{
    for (std::map<int, Client*>::iterator it = _users.begin(); it != _users.end(); ++it)
    {
        int clientFd = it->first;
        server->boolSendReply(clientFd, message, useServerPrefix);
    }
}

//-------------------- Helper Functions --------------------------

bool Channel::canJoin(const std::string& key)
{
    if (this->hasKey() && this->getKey() != key)
        return false;
    return true;

}

void Channel::addUser(int clientFd, Client* client) // Taha fixed...
{
    if (!client)
        return;
    std::pair<int, Client*> userPair(clientFd, client);
    _users.insert(userPair);
    _invited.erase(clientFd);
}

bool Channel::isTopicRestricted() const {
    return this->_topicRestricted;
}

void Channel::clearTopic() {
    this->_topic.clear();
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

void Channel::removeOperator(int clientFd)
{
    this->_operators.erase(clientFd);
}

void    Channel::kickUser(int clientFd)
{
    this->removeOperator(clientFd);
    this->removeUser(clientFd);
}