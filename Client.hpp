/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 13:55:47 by bmakhama          #+#    #+#             */
/*   Updated: 2025/05/08 14:08:51 by bmakhama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>

class Client
{
    private:
        int _fd;
        std::string _buffer;
        std::string _nickname;
        std::string _username;
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
        
        // Getter
        std::string& getBuffer();
        int getFd() const;
        bool isAuthenticated() const;
        
};