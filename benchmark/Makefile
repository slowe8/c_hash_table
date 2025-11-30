# Compiler and flags
CC = gcc
# Optimized build: no sanitizers, maximum optimization, native architecture
CFLAGS = -Wall -Wextra -Werror -std=c11 -O3 -march=native -DNDEBUG -I$(SRC_DIR)
LDFLAGS = -lrt

# Source directory
SRC_DIR = ../src

# Build directory
BUILD_DIR = ../build/benchmark

# Target executable
TARGET = $(BUILD_DIR)/perf_test

# Source files from src directory
SRC_FILES = hash_table.c hash_table_util.c
BENCH_FILE = perf_test.c

# Full source paths
SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES)) $(BENCH_FILE)

# Object files (in build directory)
OBJS = $(addprefix $(BUILD_DIR)/, $(SRC_FILES:.c=.o)) $(BUILD_DIR)/perf_test.o

# Header files
HEADERS = $(SRC_DIR)/hash_table.h $(SRC_DIR)/hash_table_util.h

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the benchmark executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files from src directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile benchmark file
$(BUILD_DIR)/perf_test.o: perf_test.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Run benchmarks
run: $(TARGET)
	$(TARGET)

# Run with verbose output
run-verbose: $(TARGET)
	@echo "Running performance tests..."
	@echo "Build flags: $(CFLAGS)"
	@$(TARGET)

# Profile with perf (Linux only)
profile: $(TARGET)
	@echo "Running with perf profiling..."
	@echo "Note: May require sudo or perf_event_paranoid adjustment"
	perf record -g $(TARGET)
	perf report

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Rebuild everything
rebuild: clean all

# Info about optimization flags
info:
	@echo "Optimization flags:"
	@echo "  -O3:          Maximum optimization"
	@echo "  -march=native: Optimize for this CPU"
	@echo "  -DNDEBUG:     Disable assertions"
	@echo "  No sanitizers for maximum performance"
	@echo ""
	@echo "To run benchmarks:"
	@echo "  make run"

# Phony targets
.PHONY: all clean run run-verbose profile rebuild info

