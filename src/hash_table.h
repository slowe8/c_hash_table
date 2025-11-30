#pragma once

#include <stddef.h>  // size_t
#include <stdlib.h>  // free
#include <string.h>  // strlen (for string wrappers)

extern const size_t HASH_TABLE_DEFAULT_INITIAL_CAPACITY;
extern const float HASH_TABLE_DEFAULT_RESIZE_THRESHOLD;
extern const float HASH_TABLE_DEFAULT_RESIZE_FACTOR;

// Function pointer type for custom value destructor
// If NULL, free() will be used
// If provided, this function will be called to clean up values
typedef void (*value_destructor_t)(void *value);

// Main hash table structure
// Contains the size, capacity, keys, key sizes, and values for the hash tables
// The hash table uses linear probing for collision resolution
// Keys are stored as c-strings, and values are stored as void pointers 
// key is copied into the hash table, so it must be a valid c-string
// value is moved into the hash table, so the hash table takes ownership of it
// and is responsible for freeing it
//
// In other words, do not free the value after inserting it (set it and any aliases to NULL to avoid UB)
//
// IMPORTANT TYPE CONSTRAINTS:
// - All values in a single hash table instance should be of the SAME TYPE
// - The destructor (if provided) applies to ALL values in the table
// - For mixed types, use separate hash tables for each type
// - For simple heap-allocated types, use NULL destructor (default free())
// - For custom types needing special cleanup, provide a custom destructor at table creation
// - insert_copy() is for trivially copyable types (primitives, simple structs)
// - insert() is for when you want move semantics (transfer ownership of heap-allocated data)
//
// GENERIC KEY SUPPORT:
// - Keys can be any trivially copyable type (strings, ints, structs, etc.)
// - Keys are stored as raw bytes and compared with memcmp
// - For string keys, use strlen(key)+1 as the key_size to include null terminator
typedef struct hash_table {
    size_t capacity;        // Capacity of the hash table
    void **keys;            // Array of keys as void* (generic byte arrays)
    size_t *key_sizes;      // Array of key sizes in bytes
    void **values;          // Array of values
    size_t size;            // Size of the hash table
    float _resize_threshold; // Resize threshold
    float _resize_factor;    // Resize factor
    value_destructor_t _value_destructor; // Custom destructor for values
} hash_table_t;

// Create a new hash table with default initial capacity (16), resize threshold (0.5), and resize factor (2.0)
// Uses free() to clean up values by default
hash_table_t *hash_table_create();

// Create a new hash table with default parameters and a custom value destructor
// Pass NULL for destructor to use free() (same as hash_table_create())
hash_table_t *hash_table_create_with_destructor(value_destructor_t destructor);

// Create a new hash table with specified initial parameters (capacity, resize threshold, resize factor)
// Uses free() to clean up values by default
//
// Using these parameters incorrectly may lead to suboptimal performance or memory issues
// Use with caution
// Probably don't use this unless you know what you are doing
hash_table_t *hash_table_create_with_parameters(size_t capacity, float resize_threshold, float resize_factor);

// Create a new hash table with all parameters including custom value destructor
// Pass NULL for destructor to use free()
//
// Using these parameters incorrectly may lead to suboptimal performance or memory issues
// Use with caution
hash_table_t *hash_table_create_with_parameters_and_destructor(size_t capacity, float resize_threshold, float resize_factor, value_destructor_t destructor);

// Destroy the hash table and free all associated memory
void hash_table_destroy(hash_table_t *table);

// Insert a key-value pair into the hash table (generic key)
// If the key already exists, update its value
//
// The key is copied into the hash table as raw bytes
// The value is moved into the hash table, so the hash table takes ownership of it
// and is responsible for freeing it
//
// Returns 0 on success, 1 on failure
// On failure, the hash table remains unchanged
int hash_table_insert(hash_table_t *table, const void *key, size_t key_size, void *value);

// Insert a key-value pair by copying the value into the hash table (generic key)
// If the key already exists, update its value
//
// The key is copied into the hash table as raw bytes
// The value is copied into the hash table (shallow copy using memcpy)
// The hash table allocates memory for the copy and takes ownership
//
// This is convenient for trivially copyable types like primitives and simple structs
//
// Returns 0 on success, 1 on failure
// On failure, the hash table remains unchanged
int hash_table_insert_copy(hash_table_t *table, const void *key, size_t key_size, const void *value, size_t value_size);

// Retrieve the value associated with a given key (generic key)
// Returns NULL if the key does not exist
//
// The returned value is an alias of the value and should not be freed directly
void *hash_table_get(hash_table_t *table, const void *key, size_t key_size);

// Remove a key-value pair from the hash table (generic key)
// Does nothing if the key does not exist
//
// Frees the key and value associated with the key
// Any remaining aliases to the value will become dangling pointers
void hash_table_remove(hash_table_t *table, const void *key, size_t key_size);

// Check if the hash table contains a given key (generic key)
// Returns 1 if the key exists, 0 otherwise
char hash_table_contains(hash_table_t *table, const void *key, size_t key_size);

// Get the current size of the hash table
size_t hash_table_size(hash_table_t *table);

// Get the current capacity of the hash table
size_t hash_table_capacity(hash_table_t *table);

// Clear all entries in the hash table
// Frees all keys and values
// Any remaining aliases to the values will become dangling pointers
void hash_table_clear(hash_table_t *table);

// ============================================================================
// String Key Convenience Functions
// ============================================================================
// These functions are specifically for null-terminated string keys
// They automatically calculate key_size as strlen(key) + 1

// Insert a key-value pair using a null-terminated string key
// Convenience wrapper for hash_table_insert that handles string length automatically
int hash_table_insert_string(hash_table_t *table, const char *key, void *value);

// Insert a key-value pair by copying using a null-terminated string key
// Convenience wrapper for hash_table_insert_copy that handles string length automatically
int hash_table_insert_copy_string(hash_table_t *table, const char *key, const void *value, size_t value_size);

// Retrieve the value associated with a null-terminated string key
// Convenience wrapper for hash_table_get that handles string length automatically
void *hash_table_get_string(hash_table_t *table, const char *key);

// Remove a key-value pair using a null-terminated string key
// Convenience wrapper for hash_table_remove that handles string length automatically
void hash_table_remove_string(hash_table_t *table, const char *key);

// Check if the hash table contains a null-terminated string key
// Convenience wrapper for hash_table_contains that handles string length automatically
char hash_table_contains_string(hash_table_t *table, const char *key);


