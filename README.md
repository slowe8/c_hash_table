# Hash Table

A performant, memory-safe hash table implementation in C with **generic key support**, custom destructor support, and flexible APIs for different use cases.

Largely inspired by [Ben Hoyt's "How to implement a hash table (in C)"](https://benhoyt.com/writings/hash-table-in-c/) blog post.

## Features

- **Generic keys** - Support for any trivially copyable key type (strings, integers, structs, etc.)
- **Linear probing** collision resolution with FNV-1a hash function
- **Automatic resizing** with configurable load factor and growth factor
- **Custom destructors** for complex types requiring special cleanup
- **Copy semantics** for trivially copyable types (primitives, simple structs)
- **Move semantics** for heap-allocated data (transfer ownership)
- **String convenience functions** for common string-key operations
- **Type-safe design** with clear ownership semantics
- **Memory safety** verified with AddressSanitizer
- **Comprehensive test suite** with 30+ tests

## Project Structure

```
hash_table/
├── src/
│   ├── hash_table.c          # Core implementation
│   ├── hash_table.h          # Public API
│   ├── hash_table_util.c     # Utility functions
│   └── hash_table_util.h     # Utility headers
├── example/
│   ├── main.c                # Usage examples (various key types)
│   └── Makefile              # Build example
├── test/
│   ├── test_hash_table.c     # Test suite (30+ tests)
│   ├── test_framework.h      # Minimal test framework
│   └── Makefile              # Build and run tests
├── benchmark/
│   ├── perf_test.c           # Performance benchmarks
│   └── Makefile              # Optimized build
└── build/                    # Build artifacts (generated)
```

## Building

### Requirements
- GCC or Clang compiler
- Make
- C11 standard library

### Build Example
```bash
cd example
make
make run
```

### Build and Run Tests
```bash
cd test
make test              # Run all tests together
make test-each         # Run each test individually
make test-test_resize  # Run specific test
make list-tests        # List all available tests
```

## Quick Start

### Basic Usage with String Keys

```c
#include "hash_table.h"

// Create a hash table
hash_table_t *table = hash_table_create();

// Insert primitives using string convenience functions
int age = 25;
hash_table_insert_copy_string(table, "age", &age, sizeof(int));

// Retrieve values
int *retrieved = (int *)hash_table_get_string(table, "age");
printf("Age: %d\n", *retrieved);

// Clean up (frees all entries automatically)
hash_table_destroy(table);
```

### Using Integer Keys

```c
// Store user data by ID
int user_id = 12345;
hash_table_insert_copy(table, &user_id, sizeof(int), user_data, sizeof(user_data_t));

// Lookup by ID
user_data_t *user = (user_data_t *)hash_table_get(table, &user_id, sizeof(int));
```

### Using Struct Keys

```c
typedef struct {
    int x;
    int y;
} Point;

// Use coordinates as keys
Point origin = {0, 0};
hash_table_insert(table, &origin, sizeof(Point), value);

// Lookup by coordinates
void *result = hash_table_get(table, &origin, sizeof(Point));
```

### Using Move Semantics

```c
// Create heap-allocated data
int *value = malloc(sizeof(int));
*value = 42;

// Insert with string convenience function - hash table takes ownership
hash_table_insert_string(table, "key", value);

// Don't free value - hash table owns it now!
```

### Custom Types with Destructors

```c
typedef struct {
    char *name;
    int *data;
    size_t data_size;
} CustomData;

// Custom cleanup function
void custom_destructor(void *ptr) {
    CustomData *obj = (CustomData *)ptr;
    free(obj->name);
    free(obj->data);
    free(obj);
}

// Create table with custom destructor
hash_table_t *table = hash_table_create_with_destructor(custom_destructor);

// Insert custom data - destructor will be called on cleanup
CustomData *data = malloc(sizeof(CustomData));
data->name = strdup("Example");
data->data = malloc(10 * sizeof(int));
data->data_size = 10;
hash_table_insert_string(table, "mydata", data);

// Destructor automatically called on destroy
hash_table_destroy(table);
```

## API Reference

### Key Types and Macros

The hash table supports **generic keys** - any trivially copyable type can be used as a key.

**Generic API**: Pass `(const void *key, size_t key_size)` for any key type
```c
int id = 42;
hash_table_insert(table, &id, sizeof(int), value);
```

**String Convenience Functions**: For null-terminated strings, use `_string` suffix functions
```c
hash_table_insert_string(table, "key", value);  // Automatically uses strlen(key)+1
```

**SKEY Macro**: Helper macro for string keys with generic API
```c
#define SKEY(str) (str), strlen(str) + 1

hash_table_insert(table, SKEY("key"), value);  // Expands to: table, "key", strlen("key")+1
```

### Creation Functions

```c
// Create with defaults (capacity=16, threshold=0.5, factor=2.0)
hash_table_t *hash_table_create();

// Create with custom destructor
hash_table_t *hash_table_create_with_destructor(value_destructor_t destructor);

// Create with custom parameters
hash_table_t *hash_table_create_with_parameters(
    size_t capacity, 
    float resize_threshold, 
    float resize_factor
);

// Full control
hash_table_t *hash_table_create_with_parameters_and_destructor(
    size_t capacity, 
    float resize_threshold, 
    float resize_factor, 
    value_destructor_t destructor
);
```

### Insertion Functions

**Generic API** (for any key type):
```c
// Insert with move semantics (hash table takes ownership)
int hash_table_insert(hash_table_t *table, const void *key, size_t key_size, void *value);

// Insert with copy semantics (for trivially copyable types)
int hash_table_insert_copy(hash_table_t *table, const void *key, size_t key_size,
                           const void *value, size_t value_size);
```

**String Convenience API** (for null-terminated string keys):
```c
// Insert with move semantics (string keys)
int hash_table_insert_string(hash_table_t *table, const char *key, void *value);

// Insert with copy semantics (string keys)
int hash_table_insert_copy_string(hash_table_t *table, const char *key,
                                  const void *value, size_t value_size);
```

### Query Functions

**Generic API**:
```c
// Retrieve value by key (returns NULL if not found)
void *hash_table_get(hash_table_t *table, const void *key, size_t key_size);

// Check if key exists
char hash_table_contains(hash_table_t *table, const void *key, size_t key_size);

// Get table metrics
size_t hash_table_size(hash_table_t *table);
size_t hash_table_capacity(hash_table_t *table);
```

**String Convenience API**:
```c
// Retrieve value by string key
void *hash_table_get_string(hash_table_t *table, const char *key);

// Check if string key exists
char hash_table_contains_string(hash_table_t *table, const char *key);
```

### Modification Functions

**Generic API**:
```c
// Remove entry (frees key and value)
void hash_table_remove(hash_table_t *table, const void *key, size_t key_size);

// Clear all entries (frees all keys and values)
void hash_table_clear(hash_table_t *table);

// Destroy table (frees everything)
void hash_table_destroy(hash_table_t *table);
```

**String Convenience API**:
```c
// Remove entry by string key
void hash_table_remove_string(hash_table_t *table, const char *key);
```

## Important Design Notes

### Type Homogeneity
All values in a single hash table instance should be of the **same type**. The destructor applies to ALL values in the table.

For different types, use separate hash tables:
```c
hash_table_t *int_table = hash_table_create();      // For integers
hash_table_t *string_table = hash_table_create();   // For strings
hash_table_t *custom_table = hash_table_create_with_destructor(my_destructor);
```

### Memory Ownership
- **Keys**: Copied into the hash table (you can free the original)
- **Values**: Ownership transferred to hash table (don't free after insert)
- **Destructor**: Called on remove, update, clear, and destroy operations

### Thread Safety
This implementation is **not thread-safe**. Use external synchronization if accessing from multiple threads.

## Performance Characteristics

- **Insert**: O(1) average, O(n) worst case (during resize)
- **Lookup**: O(1) average, O(n) worst case (with many collisions)
- **Remove**: O(1) average, O(n) worst case
- **Space**: O(n) where n is the capacity

Default configuration (threshold=0.5, factor=2.0) provides good balance between memory usage and performance.

## Testing

The test suite includes:
- Basic operations (create, insert, get, remove)
- Generic key types (strings, integers, structs)
- Edge cases (NULL handling, empty keys, long keys)
- Collision handling
- Resize behavior
- Custom destructors
- Memory leak detection (AddressSanitizer)
- Stress tests (1000+ entries)

All tests pass with zero memory leaks under AddressSanitizer.

### Performance Testing

Run performance benchmarks on optimized builds:
```bash
cd benchmark
make               # Build optimized version (no sanitizers, -O3 -march=native)
make run           # Run performance tests
```

Performance tests measure:
- Insertion throughput (operations/second)
- Lookup throughput (operations/second)
- Memory efficiency
- Scaling behavior (1K, 10K, 100K entries)

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please ensure:
- All tests pass (`make test-each` in test/)
- No memory leaks (AddressSanitizer clean)
- Code follows existing style
- New features include tests

