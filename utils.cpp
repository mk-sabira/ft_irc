#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <cerrno>  // For errno, EAGAIN, EWOULDBLOCK
#include <iostream> // For std::cout, std::cerr
#include <cstring>  // For strerror

// ------------ helper functions ----------

std::string vecToStr(std::vector<std::string> vec) // Taha changed form string& to string
{
    std::string str;

    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin())
            str += " ";
        str += *it;
    }
    return (str);
}

bool stringToInt(const std::string& str, int& result)
{
    std::istringstream iss(str);  // Create a string stream from the input string
    iss >> result;                // Try to extract an int from the string

    // Check for conversion failure or extra characters
    if (iss.fail() || !iss.eof())
        return false;             // Conversion failed or leftover characters exist

    return true;                  // Successful conversion
}
