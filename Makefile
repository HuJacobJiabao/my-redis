CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -MMD -MP -O2

SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = my_redis_server

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

rebuild: clean all

run: all
	./$(TARGET)

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(TARGET)

-include $(BUILD_DIR)/*.d
