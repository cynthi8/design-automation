CC := g++
SRC_DIR := ./src
OBJ_DIR := ./src
CPPFLAGS := -I. -Wall -Werror -g -O0

DEPS := $(wildcard $(SRC_DIR)/*.hpp)
#SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
SRC_FILES := $(SRC_DIR)/Graph.cpp $(SRC_DIR)/Placement.cpp $(SRC_DIR)/Main.cpp

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
EXEC := main

$(EXEC): $(OBJ_FILES)
	$(CC) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

clean:
	rm -rf $(OBJ_FILES) $(EXEC)