CXX = c++
CPPFLAGS = -Iincludes
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = ircserv

SRCS = src/main.cpp \
		src/utils.cpp \
		src/server.cpp \
		src/channel.cpp \
		src/client.cpp \
		src/parser.cpp \
		src/cap.cpp \
		src/join.cpp \
		src/invite.cpp \
		src/kick.cpp \
		src/topic.cpp \
		src/mode.cpp \
		src/user.cpp \
		src/pass.cpp \
		src/nick.cpp \
		src/ping.cpp \
		src/quit.cpp \
		src/notice.cpp \
		src/privmsg.cpp


OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
