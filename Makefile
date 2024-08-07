NAME	=	ircserv
CC	=		c++ #clang++  #g++-11
FLAGS	=	-Wall -Wextra -Werror # -std=c++17 

SRC		=	main.cpp src/Server.cpp src/Client.cpp src/Channel.cpp \
			src/commands/Invite.cpp src/commands/Join.cpp src/commands/Mode.cpp \
			src/commands/Topic.cpp src/commands/Quit.cpp src/commands/Part.cpp \
			src/commands/Kick.cpp  src/commands/Privmsg.cpp src/commands/Pass.cpp \
			src/commands/User.cpp src/commands/Nick.cpp

OBJ		= $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp 
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
