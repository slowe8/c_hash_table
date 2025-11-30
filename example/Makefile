# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -flto -fsanitize=address -I$(SRC_DIR)
DEBUG_FLAGS = -g -DDEBUG
LDFLAGS = -flto -fsanitize=address

# Source directory
SRC_DIR = ../src

# Build directory
BUILD_DIR = ../build

# Target executable
TARGET = $(BUILD_DIR)/hash_table_demo

# Source files
SRC_FILES = hash_table.c hash_table_util.c
EXAMPLE_FILE = main.c

# Full source paths
SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES)) $(EXAMPLE_FILE)

# Object files (in build directory)
OBJS = $(addprefix $(BUILD_DIR)/, $(SRC_FILES:.c=.o)) $(BUILD_DIR)/main.o

# Header files
HEADERS = $(SRC_DIR)/hash_table.h $(SRC_DIR)/hash_table_util.h

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files from src directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main.c from example directory
$(BUILD_DIR)/main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) 

# Run the program
run: $(TARGET)
	./$(TARGET)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all clean run rebuild debug
