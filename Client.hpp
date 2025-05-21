/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrhelmy <mrhelmy@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:55:47 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/17 17:39:06 by mrhelmy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
    private:
        int _fd;
        std::string _buffer;
        std::string _nickname;
        std::string _username;
        std::string _realname; //Stores the client's realname (from USER).
        std::string _hostname; // Taha trying to fix lime chat
        bool _registered;
        bool _authenticated;
    
    public:
        Client();
        ~Client();
        Client(const Client& copy);
        Client& operator=(const Client& obj);

        //setters
        void setBuffer(const std::string& buffer);
        void setFd(int fd);
        void setAuthenticated(bool authenticated);
        void setNickname(const std::string& nickname);
        void setUsername(const std::string& username);
        void setRealname(const std::string& realname);
        void setRegistered(bool registered);
        void setHostname(const std::string& hostname); // Taha trying to fix lime chat
        
        // Getter
        std::string& getBuffer();
        int getFd() const;
        bool isAuthenticated() const;
        std::string& getNickname();
        std::string& getUsername();
        std::string& getRealname();
        bool isRegistered() const;
        // Dina Channel
		std::string    getPrefix()const;
};

#endif