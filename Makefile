NAME		=	webserv

CPP         =  c++

# Compiler Flags
CPPFLAGS    =   -g -std=c++98 -Wall -Wextra -Werror -Wshadow -Wno-shadow -I. -Iinclude
RM          =   rm -rf

# Directories
OBJS_DIR 	=	objs
SRC_DIRS 	=	srcs

# Source Files
SRCS		=	main.cpp $(SRC_DIRS)/Socket.cpp srcs/Client.cpp $(SRC_DIRS)/Utils.cpp\
				srcs/Get.cpp srcs/Logger.cpp srcs/Config.cpp

# Object Files
OBJS		=	$(SRCS:%.cpp=$(OBJS_DIR)/%.o)

# Default target
all: $(OBJS_DIR) $(NAME)

# Link
$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@
	@echo "✅ Compilation successful."

# Compile source to object files
$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) -c $< -o $@

# Create object directory
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# Clean object files
clean:
	$(RM) $(OBJS_DIR)
	@echo "🧹 Object files cleaned."

# Clean all
fclean: clean
	$(RM) $(NAME)
	@echo "🗑️ Executable removed."

# Recompile everything
re: fclean all
