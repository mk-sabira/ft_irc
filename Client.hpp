/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:55:47 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/30 11:24:35 by bmakhama         ###   ########.fr       */
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
        std::string _realname;
        std::string _hostname;
        bool _registered;
        bool _authenticated;
    
        Client(const Client& copy);
        Client& operator=(const Client& obj);
    public:
        Client();
        ~Client();

        //setters
        void setBuffer(const std::string& buffer);
        void setFd(int fd);
        void setAuthenticated(bool authenticated);
        void setNickname(const std::string& nickname);
        void setUsername(const std::string& username);
        void setRealname(const std::string& realname);
        void setRegistered(bool registered);
        void setHostname(const std::string& hostname);
        
        // Getter
        std::string& getBuffer();
        int getFd() const;
        bool isAuthenticated() const;
        std::string& getNickname();
        std::string& getUsername();
        std::string& getRealname();
        bool isRegistered() const;
		std::string    getPrefix()const;
        std::string getHostname() const;
};

#endif