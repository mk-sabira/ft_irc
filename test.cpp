#include <iostream>      // For standard input/output (e.g., std::cout, std::cerr)
#include <sys/socket.h>  // For socket functions (e.g., socket(), bind(), listen(), accept())
#include <netinet/in.h>   // For sockaddr_in structure, which is used for specifying socket addresses
#include <arpa/inet.h>    // For functions like inet_pton() to convert IP addresses
#include <unistd.h>       // For the close() function to close file descriptors (like the socket)

class Server {
private:
    int _server_fd;                    // Socket file descriptor (used to identify the socket)
    struct sockaddr_in _server_addr;   // Holds the server's address information (IP address and port)

public:
    Server() : _server_fd(-1) {}        // Constructor: Initializes _server_fd to -1 (invalid)
    ~Server() {
        if (_server_fd != -1) {         // Destructor: Close the socket if it's valid
            close(_server_fd);          // Closes the socket to free resources
        }
    }

    bool setup(int port) {              // Setup method to create the socket and bind it to a port
        // 1. Create the socket using socket() function
        _server_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET for IPv4, SOCK_STREAM for TCP
        if (_server_fd == -1) {              // If socket creation fails
            std::cerr << "Socket creation failed\n";  // Print an error message
            return false;                  // Return false to indicate failure
        }
        std::cout << "Socket created successfully\n"; // Print success message
        return true;                       // Return true to indicate success
    }
};

int main() {
    Server server;                        // Create a Server object
    if (!server.setup(6667)) {             // Call the setup method with port 6667
        return 1;                          // If setup fails, return 1 to indicate error
    }
    return 0;                              // Return 0 to indicate success
}
