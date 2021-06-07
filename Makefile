############################ SETTINGS #########################################
###############################################################################
NAME = webserv
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(EXTRA_DEFINES) $(EXTRA_FLAGS)
CXX = clang++
IFLAGS = -I $(INCLUDE_PATH)
LDFLAGS =
###############################################################################
###############################################################################


################## FOR COMPILATION WITH DEBUG #################################
###############################################################################
# NOTE: redefining EXTRA_FLAGS and EXTRA_DEFINES

ifeq ($(TRACE_TARGET), )
	EXTRA_DEFINES+=
else
	EXTRA_DEFINES+= -D TRACE_TARGET=$(TRACE_TARGET)
endif

ifeq ($(DEBUG), )
	EXTRA_DEFINES+=
else
	EXTRA_DEFINES+= -D DEBUG=$(DEBUG)
endif

ifeq ($(flags), )
	EXTRA_FLAGS =
	VALGRIND 	=
else ifeq ($(flags), f)
	EXTRA_FLAGS +=  -fsanitize=address,undefined -g3 -ferror-limit=1
	VALGRIND 	=
else ifeq ($(flags), v)
	EXTRA_FLAGS += -g3
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
LIBRARIES = ./libraries/
###############################################################################
###############################################################################


########################### SRC/OBJ ###########################################
###############################################################################
SRC_FILES =	mime_types\
			parser\
			Server\
			Service\
			request_reader\
			request_parser\
			get_service\
			delete_service\
			put_service\
			utils\
			Service_generator\
			string\
			http_methods\
			cgi_service\
			debug_functions\
			Error_service\
			Services_hardcoded_body\
			Services_hardcoded_status_line\

# HERE SRC_FILES

SRC := $(patsubst %, $(SRC_PATH)%.cpp, $(SRC_FILES))
OBJ := $(patsubst %, $(OBJ_PATH)%.o, $(basename $(notdir $(SRC))))

OBJ_MAIN += $(OBJ_PATH)main.o
###############################################################################
###############################################################################

########################### HEADER FILES ######################################
###############################################################################
HPP_FILES = webserver\
			parser\
			exceptions\
			server\
			http_methods\
			Service\
			request_reader\
			request_parser\
			get_service\
			delete_service\
			put_service\
			utils\
			Service_generator\
			string\
			debug_functions\
			debug_and_traces_macros\
			cgi_service\
			Error_service\
			Services_hardcoded_body\
			Services_hardcoded_status_line\

# HERE HPP_FILES

INCLUDES += $(patsubst %, $(INCLUDE_PATH)%.hpp, $(HPP_FILES))
###############################################################################
###############################################################################

########################### DEPENDENCIES ######################################
###############################################################################
# Dependencies:
DEPS = $(INCLUDES)
#ADD EXTRANEOUS DEPENCENCIES LIKE LIBRARY INCLUDES.
LIB_REGEX = $(LIBRARIES)libregex/libregex.a
LEXER = $(LIBRARIES)lexer/lexer.a
DEPS += $(LIB_REGEX) $(LEXER)
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

.PHONY: all clean fclean re break_implicit_rule _multibyte_forwarder _byte_forwarder

$(OBJ_PATH):
	@mkdir -p $(OBJ_PATH)
	@echo "\t\t$(CREAT_FG)created the $(OBJECTS_BG)$(BLACK_FG) $@ $(BLACK_BG)$(CREAT_FG) repository for $(NAME_BG)$(BLACK_FG) $(NAME) $(BLACK_BG)$(CREAT_FG)$(CLEAR_COLOR)"

$(NAME): $(DEPS) $(OBJ_MAIN) $(OBJ)
	@$(CXX) $(CXXFLAGS) $(IFLAGS) $(OBJ_MAIN) $(OBJ) $(LIB_REGEX) $(LEXER) -o $@ $(LDFLAGS)
	@echo "\t\t$(CREAT_FG)Binary $(NAME_BG)$(BLACK_FG) $(NAME) $(BLACK_BG)$(CREAT_FG) has been created$(CLEAR_COLOR)"
	@echo "\t\t$(CREAT_FG)We used the flags: $(CXXFLAGS)$(CLEAR_COLOR)\n"

$(OBJ_MAIN): $(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(INCLUDES) | $(OBJ_PATH)
	@$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@
	@printf "[\e[32mOK\e[0m] %s\n" $@

$(OBJ): $(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(INCLUDES) | $(OBJ_PATH)
	@$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@
	@printf "[\e[32mOK\e[0m] %s\n" $@

$(LEXER): break_implicit_rule
	@make -C $(LIBRARIES)/lexer
	@echo "\t\t$(CREAT_FG)Static Library $(NAME_BG)$(BLACK_FG) $@ $(BLACK_BG)$(CREAT_FG) has been created$(CLEAR_COLOR)"

$(LIB_REGEX): break_implicit_rule
	@make -C $(LIBRARIES)/libregex
	@echo "\t\t$(CREAT_FG)Static Library $(NAME_BG)$(BLACK_FG) $@ $(BLACK_BG)$(CREAT_FG) has been created$(CLEAR_COLOR)"

break_implicit_r:

_byte_forwarder:
	@gcc $(SRC_PATH)byte_forwarder_test.cpp -o $@
	@echo "\t\t$(CREAT_FG)Binary $(NAME_BG)$(BLACK_FG) $@ $(BLACK_BG)$(CREAT_FG) has been created$(CLEAR_COLOR)"

_multibyte_forwarder:
	@$(CXX) $(CXXFLAGS) $(SRC_PATH)multibyte_forwarder_test.cpp -o $@
	@echo "\t\t$(CREAT_FG)Binary $(NAME_BG)$(BLACK_FG) $@ $(BLACK_BG)$(CREAT_FG) has been created$(CLEAR_COLOR)"

_test_variable_changements:
	$(eval CXXFLAGS = -std=c++11 -w)
#this will silent the warning as well

clean:
	@echo "\t\t$(REMOVE_FG)deleting $(OBJECTS_BG)$(BLACK_FG) $(OBJ_PATH) $(BLACK_BG)$(REMOVE_FG) containing all the .o files for $(NAME_BG)$(BLACK_FG) $(NAME) $(CLEAR_COLOR)"
	@rm -rf $(OBJ_PATH)
	@rm -rf _byte_forwarder
	@rm -rf _multibyte_forwarder
#	@make clean -C $(LIBRARIES)/libregex
#	@make clean -C $(LIBRARIES)/lexer

fclean: clean
	@echo "\t\t$(REMOVE_FG)deleting $(NAME_BG)$(BLACK_FG) $(NAME) $(BLACK_BG)$(REMOVE_FG)...$(CLEAR_COLOR)"
	@rm -rf $(NAME)
	@make fclean -C $(LIBRARIES)/libregex
	@make fclean -C $(LIBRARIES)/lexer
	@echo "\t\t$(REMOVE_FG)deleting $(NAME_BG)$(BLACK_FG) compile_commands.json $(BLACK_BG)$(REMOVE_FG)...$(CLEAR_COLOR)"
	@rm -f compile_commands.json

re: fclean all
