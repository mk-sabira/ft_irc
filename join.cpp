#include "channel.hpp"

/*
- check channel existance
- chaeck the [invite-only, key"password", user limit]
- add user to the map
- broadcast JOIN to other users
- send topic and users list to the new user
*/

void    joinCommand(int userFd, std::string channelName, std::string key)
{
    /*
        // check if channel doesnt exist:
            - if cant be created -> error 
            - create new channel
            - the user will be the operator of the channel
    */
//    -------------------------------------------------------------------------
   /*
        // check if channel exists:
            - if user already in channel return
            - is the channel invite-only
                - if yes and the user isn't invited return with an error, else continue
            - does the channel has a password
                - if yes, compare the key given by the user, if wrong reject 
            - is the channel full
                - if yes reject, else add user to the map
    */

    // after join successed
    /*
        - the server sends a broadcast to all users in the channel including the new user
        - send the current topic and users list of the channel to the new user
        - 
    */

}