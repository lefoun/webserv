# SRCS = webserv.cc ../parser/config_parser.cc
SRCS = beejserv.cc

GREEN        = \033[1;32m
RESET		 = \033[0m

CC = c++
INCLUDES = 
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
NAME = server 

ifeq ($(NOFLAGS), 1)
	FLAGS = 
else
	FLAGS = -std=c++98 -pedantic
#	#  FLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
endif

ifeq ($(debug), 1)
	FLAGS += -g
	PRINTED_MESSAGE = "$(GREEN)Buckle up we're in Debug mode chief$(RESET)"
else
#	FLAGS += -D NDEBUG
	PRINTED_MESSAGE = "$(GREEN)Everything was made successfuly chief$(RESET)"
endif

all:	$(NAME)

$(NAME):	$(OBJS)
	@$(CC) $(FLAGS) $(OBJS) -o $(NAME) 
	@echo $(PRINTED_MESSAGE)

%.o:	%.cc
	@$(CC) -MP -MMD -c $(FLAGS) $< -o $@ 

-include $(DEPS)

clean:
	@rm -rf $(OBJS) $(DEPS)

fclean:
	@rm -rf $(NAME) $(OBJS) $(DEPS)

re: fclean all

.PHONY: clean re all