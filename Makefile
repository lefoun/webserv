#SRCS = webserv.cc helper_functions.cc parser/config_parser.cc parser/handle_directives.cc parser/parser_utils.cc request_parsing.cc choose_location.cc return_code_pages_40x_5xx.cc
SRCS = srcs/webserv.cc \
	srcs/server/cgi.cc srcs/server/make_response.cc srcs/server/return_code_pages_40x_5xx.cc\
	srcs/server/choose_location.cc srcs/server/request_parsing.cc srcs/server/sockets.cc srcs/server/helper_functions.cc\
	srcs/parser/config_parser.cc srcs/parser/handle_directives.cc srcs/parser/parser_utils.cc
GREEN        = \033[1;32m
RESET		 = \033[0m

CC = c++
INCLUDES = includes/
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
NAME = webserv
FLAGS = -std=c++98 -pedantic
ifeq ($(NOFLAGS), 1)
	FLAGS =
else
	FLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
endif

ifeq ($(debug), 1)
	FLAGS += -g -fstandalone-debug
	PRINTED_MESSAGE = "$(GREEN)Buckle up we're in Debug mode chief$(RESET)"
else
#	FLAGS += -D NDEBUG
	PRINTED_MESSAGE = "$(GREEN)Everything was made successfuly chief$(RESET)"
endif

all:	$(NAME)

$(NAME):	$(OBJS)
	@$(CC) $(FLAGS) $(OBJS) -o $(NAME) -Iincludes
	@echo $(PRINTED_MESSAGE)

%.o:	%.cc
	@$(CC) -MP -MMD -c $(FLAGS) $< -o $@ -Iincludes

-include $(DEPS)

clean:
	@rm -rf $(OBJS) $(DEPS)

fclean:
	@rm -rf $(NAME) $(OBJS) $(DEPS)

re: fclean all

.PHONY: clean re all fclean
