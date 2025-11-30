# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -g -fsanitize=address -I../src
LDFLAGS = -fsanitize=address

# Source directory
SRC_DIR = ../src

# Build directory
BUILD_DIR = ../build/test

# Target executable
TARGET = $(BUILD_DIR)/test_hash_table

# Source files from src directory
SRC_FILES = hash_table.c hash_table_util.c
TEST_FILE = test_hash_table.c

# Full source paths
SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES)) $(TEST_FILE)

# Object files (in build directory)
OBJS = $(addprefix $(BUILD_DIR)/, $(SRC_FILES:.c=.o)) $(BUILD_DIR)/test_hash_table.o

# Header files
HEADERS = $(SRC_DIR)/hash_table.h $(SRC_DIR)/hash_table_util.h test_framework.h

# Individual test names (extracted from test file)
TESTS = test_create_and_destroy \
        test_create_with_custom_capacity \
        test_insert_and_get_single \
        test_insert_copy_primitives \
        test_insert_multiple_values \
        test_update_existing_key \
        test_get_nonexistent_key \
        test_contains \
        test_remove_existing_key \
        test_remove_nonexistent_key \
        test_remove_multiple \
        test_clear \
        test_resize \
        test_null_table_operations \
        test_null_key_operations \
        test_empty_key \
        test_long_keys \
        test_collision_handling \
        test_custom_destructor \
        test_custom_destructor_on_update \
        test_custom_destructor_on_remove \
        test_large_dataset

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the test executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files from src directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test file
$(BUILD_DIR)/test_hash_table.o: test_hash_table.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Run all tests
test: $(TARGET)
	$(TARGET)

# Run each test individually in sequence, stopping on first failure
test-each: $(TARGET)
	@echo "Running each test individually..."
	@for test in $(TESTS); do \
		echo ""; \
		echo "========================================"; \
		echo "Running: $$test"; \
		echo "========================================"; \
		TEST_FILTER=$$test $(TARGET); \
		if [ $$? -ne 0 ]; then \
			echo ""; \
			echo "*** Test $$test FAILED ***"; \
			exit 1; \
		fi; \
	done
	@echo ""
	@echo "========================================"
	@echo "All individual tests passed!"
	@echo "========================================"

# Run individual tests (usage: make test-NAME)
$(addprefix test-, $(TESTS)): test-%: $(TARGET)
	@echo "Running test: $*"
	@TEST_FILTER=$* $(TARGET) || (echo "Test $* failed" && exit 1)

# List all available tests
list-tests:
	@echo "Available tests:"
	@for test in $(TESTS); do echo "  - $$test (run with: make test-$$test)"; done

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all clean test rebuild list-tests $(addprefix test-, $(TESTS))
