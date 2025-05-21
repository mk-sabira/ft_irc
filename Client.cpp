/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrhelmy <mrhelmy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:59:47 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/18 00:11:10 by mrhelmy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() {
    _fd = -1;
    _buffer = "";
    _nickname = "";
    _username = "";
    _realname = ""; //Stores the client's realname (from USER).
    _registered = false;
    _authenticated = false;
}

Client::~Client() {}

Client::Client(const Client &copy)
{
    (void)copy;
    // all the atrubutes must be copied
}

Client &Client::operator=(const Client &obj)
{
    (void)obj;
    // must be reassigned
    return (*this);
}

// getters
std::string &Client::getBuffer()
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

// std::string Client::getPrefix() const  // Dina channel // commented by Taha uncomment later when we get a host
// {
//     return this->_nickname + "!" + this->_username + "@" + this->_hostname;
// }

std::string Client::getPrefix() const
{
    return _nickname + "!" + _username + "@" + _hostname;
}

// setters
void Client::setBuffer(const std::string &buffer)
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
    _nickname = nickname;
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

// Taha trying to fix lime chat
void Client::setHostname(const std::string& hostname)
{
    _hostname = hostname;
}
