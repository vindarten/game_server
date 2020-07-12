CC = gcc
CFLAGS += -Wall -g
CXXFLAGS += -Wall -g

SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin
INCLUDE_DIR = ./include

CFLAGS += -I$(INCLUDE_DIR)
CXXFLAGS += -I$(INCLUDE_DIR)

SRCMODULES = lexeme.cpp tables.cpp automat.cpp GameInfo.cpp IpnEx.cpp IpnElem.cpp Ipn.cpp StackOperations.cpp Parser.cpp MyString.cpp

OBJFILES = $(addprefix $(OBJ_DIR)/, $(SRCMODULES:.cpp=.o))

all: directories server robot

directories:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

server: directories $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) $(lastword $^) -o $(BIN_DIR)/$@

robot: directories $(SRC_DIR)/robot.cpp $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(wordlist 2, $(words $^), $^) -o $(BIN_DIR)/$@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include dependence.mk
endif

dependence.mk: $(SRCMODULES)
	$(CXX) -MM $^ > $@

clean:
	rm -rvf $(BIN_DIR) $(OBJ_DIR)
