CC = c++

NAME := Matt_daemon

CPPFLAGS := -Wall -Wextra -Werror -std=c++11

SRCS := main.cpp Tintin_reporter.cpp

OBJS := ${SRCS:.cpp=.o}

OBJDIR := $(addprefix obj/, $(OBJS))

INCLUDES := Matt_daemon.hpp Tintin_reporter.hpp

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
