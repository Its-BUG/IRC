NAME	 := ircserv
CXX		 := c++ --std=c++98 
CXXFLAGS := -W -Wextra -Werror 

SRCDIRS  := src/core src/sockets src/interfaces src/commands
SRCS	 := $(wildcard $(addsuffix /*.cpp, $(SRCDIRS)))
OBJS	 := $(patsubst src/%.cpp, objs/%.o, $(SRCS))
DEPS	 := $(OBJS:.o=.d)

.PHONY: all clean fclean re

all: $(NAME)

-include $(DEPS)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -Iinclude $^ -o $@

objs/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Iinclude -MMD -MP -c $< -o $@

clean:
	rm -rf objs/

fclean: clean
	rm -rf $(NAME)

re: fclean all