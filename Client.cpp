/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:59:47 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/08 14:07:58 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

