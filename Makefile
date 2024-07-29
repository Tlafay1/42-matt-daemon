CC = c++

NAME := matt_daemon

CPPFLAGS := -Wall -Wextra -Werror -std=c++98

SRCS := main.cpp

OBJS := ${SRCS:.cpp=.o}

OBJDIR := $(addprefix obj/, $(OBJS))

INCLUDES := matt_daemon.hpp

INCDIR := $(addprefix includes/, $(INCLUDES))

all : $(NAME)

$(NAME) : $(OBJDIR)
	$(CC) $(OBJDIR) -o $(NAME) $(CPPFLAGS)

obj/%.o : src/%.cpp $(INCDIR) Makefile
	mkdir -p obj
	$(CC) -c $< -I includes $(CPPFLAGS) -o $@

clean :
	$(RM) $(OBJDIR)

fclean : clean
	$(RM) $(NAME) 

re : fclean all

.PHONY : all clean fclean re
