CXX      ?= g++
CXXFLAGS ?= -std=c++14 -Wall -Wextra -O2
SRC      := $(filter-out test_main.cpp,$(wildcard *.cpp))
vcalc: $(SRC:.cpp=.o)
	$(CXX) -o $@ $^ -lboost_program_options
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- модульные тесты (не включают main.cpp, NetworkServer.cpp, SessionHandler.cpp) ---
TEST_SRC := test_main.cpp Authenticator.cpp Calculator.cpp ClientBase.cpp DataSerializer.cpp \
            Logger.cpp UserInterface.cpp sha256.cpp
test_vcalc: $(TEST_SRC) $(wildcard *.h)
	$(CXX) -std=c++17 -Wall -Wextra -O0 -o $@ $(TEST_SRC) -lboost_program_options
test: test_vcalc
	./test_vcalc

clean:
	rm -f *.o vcalc test_vcalc
