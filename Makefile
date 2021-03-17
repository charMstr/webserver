# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2020/01/27 17:46:05 by charmstr          #+#    #+#              #
#    Updated: 2021/03/17 13:20:50 by lspiess          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


############################ SETTINGS #########################################
###############################################################################
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CXX = clang++
NAME = webserver
IFLAGS = -I $(INCLUDE_PATH)
LDFLAGS =
###############################################################################
###############################################################################


################## FOR COMPILATION WITH DEBUG #################################
###############################################################################
ifeq ($(f), n)
CXXFLAGS 	=
VALGRIND 	=
else ifeq ($(f), d)
CXXFLAGS 	+= -D DEBUG=1
VALGRIND 	=
else ifeq ($(f), f)
CXXFLAGS 	+=  -fsanitize=address,undefined -g3 -ferror-limit=1
VALGRIND 	=
else ifeq ($(f), v)
CXXFLAGS 		+= -g3
SHOW_LEAK 	= --show-leak-kinds=definite
VALGRIND 	= valgrind --track-origins=yes --leak-check=full $(SHOW_LEAK)
endif
###############################################################################
###############################################################################


############################ PATHS ############################################
###############################################################################
OBJ_PATH = ./objects/
SRC_PATH = ./sources/
INCLUDE_PATH = ./includes/
###############################################################################
###############################################################################


########################### SRC/OBJ & BONUS FILES #############################
###############################################################################
# HERE
SRC_FILES =	main\
			Server\
			server_utils\
			request_service\
			debug_functions


SRC = $(patsubst %, $(SRC_PATH)%.cpp, $(SRC_FILES))
OBJ = $(patsubst %, $(OBJ_PATH)%.o, $(basename $(notdir $(SRC))))
###############################################################################
###############################################################################


########################### HEADER FILES ######################################
###############################################################################
#HERE
HPP_FILES = webserver\
			exceptions\
			server\
			server_utils\
			http_methods\
			Service\
			request_service

INCLUDES := $(patsubst %, $(INCLUDE_PATH)%.hpp, $(HPP_FILES))
###############################################################################
###############################################################################


########################### DEPENDENCIES ######################################
###############################################################################
# Dependencies:
DEPS = $(INCLUDES)
#ADD EXTRANEOUS DEPENCENCIES LIKE LIBRARY INCLUDES.
# DEPS += ./libft/libft.h
###############################################################################
###############################################################################


###############################################################################
############################### COLOR CODE ####################################
REMOVE_FG = \033[38;5;196m
CREAT_FG = \033[38;5;46m
BLACK_FG = \033[38;5;0m
BLACK_BG = \033[48;5;0m
CLEAR_COLOR = \033[m
NAME_BG = \033[48;5;39m
OBJECTS_BG = \033[48;5;11m
###############################################################################
###############################################################################


all: $(NAME)

.PHONY: all clean fclean re

$(OBJ_PATH):
	@mkdir -p $(OBJ_PATH)
	@echo "\t\t$(CREAT_FG)created the $(OBJECTS_BG)$(BLACK_FG) $@ $(BLACK_BG)$(CREAT_FG) repository for $(NAME_BG)$(BLACK_FG) $(NAME) $(BLACK_BG)$(CREAT_FG)$(CLEAR_COLOR)"

$(NAME): $(DEPS) $(OBJ)
	@$(CXX) $(CXXFLAGS) $(IFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	@echo "\t\t$(CREAT_FG)Binary $(NAME_BG)$(BLACK_FG) $(NAME) $(BLACK_BG)$(CREAT_FG) has been created$(CLEAR_COLOR)"
	@echo "\t\t$(CREAT_FG)We used the flags: $(CXXFLAGS)$(CLEAR_COLOR)\n"

$(OBJ): $(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(INCLUDES) | $(OBJ_PATH)
	@$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

clean:
	@echo "\t\t$(REMOVE_FG)deleting $(OBJECTS_BG)$(BLACK_FG) $(OBJ_PATH) $(BLACK_BG)$(REMOVE_FG) containing all the .o files for $(NAME_BG)$(BLACK_FG) $(NAME) $(CLEAR_COLOR)"
	@rm -rf $(OBJ_PATH)

fclean: clean
	@echo "\t\t$(REMOVE_FG)deleting $(NAME_BG)$(BLACK_FG) $(NAME) $(BLACK_BG)$(REMOVE_FG)...$(CLEAR_COLOR)"
	@rm -rf $(NAME)
	@echo "\t\t$(REMOVE_FG)deleting $(NAME_BG)$(BLACK_FG) compile_commands.json $(BLACK_BG)$(REMOVE_FG)...$(CLEAR_COLOR)"
	@rm -f compile_commands.json

re: fclean all
