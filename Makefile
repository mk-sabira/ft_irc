# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mrhelmy <mrhelmy@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/02 10:09:42 by bmakhama          #+#    #+#              #
#    Updated: 2025/05/17 17:43:26 by mrhelmy          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

C++ = c++
CXXFLAGS = -Wall -Wextra -Werror -fsanitize=address -g --std=c++98
#no falgs because of compile error
NAME = ircserv
SRC = main.cpp Server.cpp cleaningUtils.cpp Client.cpp processCmd.cpp Channel.cpp server_channel.cpp 
OBJ = ${SRC:.cpp=.o}

all: $(NAME)

$(NAME): $(OBJ)
	$(C++) $(CXXFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(C++) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re