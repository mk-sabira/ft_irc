#include "Channel.hpp"

/* syntax:
        MODE <channel> [ (+|-)<modes> [parameters] ]
*/

void Server::modeCommand(int userFd, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3)
	{
		// return the current modes.
		return ;

		/*
			Code		324
			Name		RPL_CHANNELMODEIS
			Description	Returned when a client requests to view the current modes of a channel.
			Format:		 :server 324 <nick> <channel> <modes> [params]
		*/
	}
	std::map<std::string, Channel*>::iterator it = _channels.find(tokens[1]);
    if (it == _channels.end())
    {
        sendError(userFd, ERR_NOSUCHCHANNEL, tokens[1] + " :No such channel");
        return;
    }

    Channel& channel = *(it->second);
	if (!channel.isUser(userFd))
    {
        sendError(userFd, ERR_USERONCHANNEL, tokens[1] + " :You're not on that channel");
        return;
    }
	if (!channel.isOperator(userFd))
    {
        sendError(userFd, ERR_CHANOPRIVSNEEDED, tokens[1] + " :You're not channel operator");
        return;
    }
	char	sign = tokens[2][0];
	if (sign != '+' && sign != '-')
	{
		//error 
		return ;
	}
	for (int i = 1; tokens[2][i]; i++)
	{
		char mode = tokens[2][i];
		switch (mode)
		{
			case 'i':
			{
				channel.setInviteFlag(sign);
				break;
			}
			case 't':
				channel.setRestrictions(sign);
				break;
			case 'k':
			{
				if (tokens.size() < 4 && sign == '+')
				{
					//error
					return ;
				}
				channel.setKeyMode(sign, tokens[3]);
				break;
			}
			case 'o':
			{
				if (tokens.size() < 4)
				{
					//error
					return ;
				}
				channel.setOperatorMode(sign, userFd);
				break;
			}
			case 'l':
			{
				if (tokens.size() < 4 && sign == '+')
				{
					//error
					return ;
				}
				channel.setUserLimit(sign, stoi(tokens[3]));
				break;
			}
		
			default:
			{
				//error
				return ;
				// break;
			}
		}

	}


}

