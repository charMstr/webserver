# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/02/17 18:50:28 by bccyv             #+#    #+#              #
#    Updated: 2021/03/17 19:46:42 by charmstr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ****************
#   Varriables:

LIBNAME =   lexer.a

AR      =   ar
ARFLAGS =   rc

CC      =   gcc
CFLAGS  =   -Wall -Wextra -Werror

SRCDIR  =   sources
INCDIR  =   includes
OBJDIR  =   obj
SRCS    =   $(notdir $(shell ls -1 $(SRCDIR)/*.c))
OBJS    =   $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))
DPDCS   =   $(OBJS:.o=.d)

# ****************
#   Rules:

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	@$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS)
	@printf "[\e[32mOK\e[0m] %s\n" $@

-include $(DPDCS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -MMD -I$(INCDIR) -c $< -o $@
	@printf "[\e[32mOK\e[0m] %s\n" $@

clean: _clean

fclean: _clean
ifeq ($(shell ls -1 | grep $(LIBNAME)),$(LIBNAME))
	@rm -rf $(LIBNAME)
	@printf "[\e[31mCLEAN\e[0m] %s\n" $(LIBNAME)
endif

_clean:
ifeq ($(shell ls -1 | grep $(OBJDIR)),$(OBJDIR))
	@rm -rf $(OBJS) $(DPDCS)
	@printf "[\e[31mCLEAN\e[0m] %s\n" $(OBJS)
	@rm -rf $(OBJDIR)
endif

re: fclean all
