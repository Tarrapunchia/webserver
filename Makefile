CXX       := c++
CXXFLAGS  := -std=c++98 -Wall -Werror -Wextra -g -O0 -D_GLIBCXX_DEBUG -Iinc
LDFLAGS   :=
LDLIBS    := 
SRCDIR    := src
OBJDIR    := obj
SRCS      := $(wildcard $(SRCDIR)/*.cpp)
OBJS      := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
MAIN_OBJ  := main.o
NAME      := webserv

.PHONY: all clean fclean re
all: $(NAME)

$(NAME): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(MAIN_OBJ): main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(MAIN_OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
