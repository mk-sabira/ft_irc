# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: bmakhama <bmakhama@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/02 10:09:42 by bmakhama          #+#    #+#              #
#    Updated: 2025/05/08 14:07:36 by bmakhama         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

C++ = c++
CXXFLAGS = -Wall -Wextra -Werror  -std=c++98

NAME = ircserv
SRC = main.cpp Server.cpp cleaningUtils.cpp Client.cpp
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