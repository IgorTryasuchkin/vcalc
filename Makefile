CXX      ?= g++
CXXFLAGS ?= -std=c++14 -Wall -Wextra -O2
SRC      := $(wildcard *.cpp)
vcalc: $(SRC:.cpp=.o)
	$(CXX) -o $@ $^ -lboost_program_options
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f *.o vcalc
