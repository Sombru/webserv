# Program name
NAME = webserv

# Compiler
CC    = c++
FLAGS = -g -std=c++98 -Wall -Wextra -Werror -Wshadow -Wno-shadow 

# Sources
SRC = main.cpp \
	  Socket.cpp
# Objects
OBJ_PATH = obj/
OBJ 	 = $(SRC:.cpp=.o)
OBJS	 = $(addprefix $(OBJ_PATH), $(OBJ))

# Targets
all: $(OBJ_PATH) $(NAME)

$(OBJ_PATH):
	@mkdir -p $(OBJ_PATH)

$(OBJ_PATH)%.o: %.cpp
	@$(CC) $(FLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@$(CC) $(FLAGS) $(OBJS) -o $(NAME)

clean:
	@rm -rf $(OBJ_PATH)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re