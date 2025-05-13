CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRC_DIR = src
OBJ_DIR = obj
BIN = redis_server

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/RedisServer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(BIN)

$(BIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(BIN) $(OBJ_DIR)/*.o
