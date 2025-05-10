

#include <iostream>

class Client
{
    private:
        int _fd;
        std::string _buffer;
        std::string _nickname;
        std::string _username;
        std::string _realname; //Stores the client’s realname (from USER).
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
        
        // Getter
        std::string& getBuffer();
        int getFd() const;
        bool isAuthenticated() const;
        std::string& getNickname();
        std::string& getUsername();
        std::string& getRealname();
        bool isRegistered() const;
        
};