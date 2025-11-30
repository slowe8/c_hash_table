#include "hash_table.h"
#include "hash_table_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

// Forward declarations for static functions
static int hash_table_resize(hash_table_t *table, size_t new_capacity);
static size_t hash_table_fnv_1a(const void *key, size_t key_size);

const size_t HASH_TABLE_DEFAULT_INITIAL_CAPACITY = 16;
const float HASH_TABLE_DEFAULT_RESIZE_THRESHOLD = 0.5f;
const float HASH_TABLE_DEFAULT_RESIZE_FACTOR = 2.0f;

hash_table_t *hash_table_create() {
    return hash_table_create_with_parameters_and_destructor(
        HASH_TABLE_DEFAULT_INITIAL_CAPACITY, 
        HASH_TABLE_DEFAULT_RESIZE_THRESHOLD, 
        HASH_TABLE_DEFAULT_RESIZE_FACTOR,
        NULL
    );
}

hash_table_t *hash_table_create_with_destructor(value_destructor_t destructor) {
    return hash_table_create_with_parameters_and_destructor(
        HASH_TABLE_DEFAULT_INITIAL_CAPACITY, 
        HASH_TABLE_DEFAULT_RESIZE_THRESHOLD, 
        HASH_TABLE_DEFAULT_RESIZE_FACTOR,
        destructor
    );
}

hash_table_t *hash_table_create_with_parameters(size_t capacity, float resize_threshold, float resize_factor) {
    return hash_table_create_with_parameters_and_destructor(capacity, resize_threshold, resize_factor, NULL);
}

hash_table_t *hash_table_create_with_parameters_and_destructor(size_t capacity, float resize_threshold, float resize_factor, value_destructor_t destructor) {
    hash_table_t *table = malloc(sizeof(hash_table_t));
    table->size = 0;
    table->capacity = capacity;
    table->_resize_threshold = resize_threshold;
    table->_resize_factor = resize_factor;
    table->_value_destructor = destructor;
    table->keys = calloc(table->capacity, sizeof(void *));
    table->key_sizes = calloc(table->capacity, sizeof(size_t));
    table->values = calloc(table->capacity, sizeof(void *));
    return table;
}

void hash_table_destroy(hash_table_t *table) {
    if (table == NULL) return;
    hash_table_clear(table);
    free(table->keys);
    free(table->key_sizes);
    free(table->values);
    free(table);
}

void hash_table_clear(hash_table_t *table) {
    if (table == NULL) return;
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->keys[i] != NULL) {
            free(table->keys[i]);
            if (table->_value_destructor != NULL) {
                table->_value_destructor(table->values[i]);
            } else {
                free(table->values[i]);
            }
            table->keys[i] = NULL;
            table->values[i] = NULL;
        }
    }
    table->size = 0;
}

size_t hash_table_size(hash_table_t *table) {
    if (table == NULL) return 0;
    return table->size;
}

size_t hash_table_capacity(hash_table_t *table) {
    if (table == NULL) return 0;
    return table->capacity;
}

char hash_table_contains(hash_table_t *table, const void *key, size_t key_size) {
    if (table == NULL || key == NULL) return 0;
    return hash_table_get(table, key, key_size) != NULL;
}

void *hash_table_get(hash_table_t *table, const void *key, size_t key_size) {
    if (table == NULL || key == NULL) return NULL;

    size_t index = hash_table_fnv_1a(key, key_size) % table->capacity;
    size_t iterations = 0;

    while (table->keys[index] != NULL && iterations < table->capacity) {
        if (table->key_sizes[index] == key_size && 
            memcmp(table->keys[index], key, key_size) == 0) {
            return table->values[index];
        }

        index = (index + 1) % table->capacity;
        iterations++;
    }

    return NULL;
}

static int hash_table_resize(hash_table_t *table, size_t new_capacity) {
    if (table == NULL || new_capacity <= table->capacity) return 1;

    // Save old arrays
    void **old_keys = table->keys;
    size_t *old_key_sizes = table->key_sizes;
    void **old_values = table->values;
    size_t old_capacity = table->capacity;

    // Allocate new arrays
    table->keys = calloc(new_capacity, sizeof(void *));
    table->key_sizes = calloc(new_capacity, sizeof(size_t));
    table->values = calloc(new_capacity, sizeof(void *));
    
    if (table->keys == NULL || table->key_sizes == NULL || table->values == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for resized hash table.\n");
        // Restore old arrays on failure
        free(table->keys);
        free(table->key_sizes);
        free(table->values);
        table->keys = old_keys;
        table->key_sizes = old_key_sizes;
        table->values = old_values;
        return 1;
    }

    // Update capacity and reset size
    table->capacity = new_capacity;
    table->size = 0;

    // Rehash all existing entries into the new arrays
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_keys[i] != NULL) {
            // Find new position using hash function and linear probing
            size_t index = hash_table_fnv_1a(old_keys[i], old_key_sizes[i]) % table->capacity;
            size_t iterations = 0;
            
            while (table->keys[index] != NULL && iterations < table->capacity) {
                index = (index + 1) % table->capacity;
                iterations++;
            }
            
            // Move key, key_size, and value to new arrays
            table->keys[index] = old_keys[i];
            table->key_sizes[index] = old_key_sizes[i];
            table->values[index] = old_values[i];
            table->size++;
        }
    }

    // Free old arrays (keys/values have been moved, not freed)
    free(old_keys);
    free(old_key_sizes);
    free(old_values);
    
    return 0;
}

int hash_table_insert(hash_table_t *table, const void *key, size_t key_size, void *value) {
    if (table == NULL || key == NULL) return 1;

    if ((float)(table->size + 1) / table->capacity > table->_resize_threshold) {
        if (hash_table_resize(table, (size_t)(table->capacity * table->_resize_factor)) != 0) {
            fprintf(stderr, "Error: Failed to resize hash table during insert.\n");
            return 1;
        }
    }

    size_t index = hash_table_fnv_1a(key, key_size) % table->capacity;
    size_t iterations = 0;

    while (table->keys[index] != NULL && iterations < table->capacity) {

        // If the key already exists, update its value
        if (table->key_sizes[index] == key_size && 
            memcmp(table->keys[index], key, key_size) == 0) {
            if (table->_value_destructor != NULL) {
                table->_value_destructor(table->values[index]);
            } else {
                free(table->values[index]);
            }
            table->values[index] = value;
            return 0;
        }

        index = (index + 1) % table->capacity;
        iterations++;
    }

    // Must have landed on a NULL slot, so insert new key-value pair
    void *key_copy = malloc(key_size);
    if (key_copy == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for key.\n");
        return 1;
    }
    memcpy(key_copy, key, key_size);
    
    table->keys[index] = key_copy;
    table->key_sizes[index] = key_size;
    table->values[index] = value;
    table->size++;
    return 0;
}

void hash_table_remove(hash_table_t *table, const void *key, size_t key_size) {
    if (table == NULL || key == NULL) return;

    size_t index = hash_table_fnv_1a(key, key_size) % table->capacity;
    size_t iterations = 0;

    while (table->keys[index] != NULL && iterations < table->capacity) {
        if (table->key_sizes[index] == key_size &&
            memcmp(table->keys[index], key, key_size) == 0) {
            free(table->keys[index]);
            if (table->_value_destructor != NULL) {
                table->_value_destructor(table->values[index]);
            } else {
                free(table->values[index]);
            }
            table->keys[index] = NULL;
            table->key_sizes[index] = 0;
            table->values[index] = NULL;
            table->size--;
            return;
        }

        index = (index + 1) % table->capacity;
        iterations++;
    }
}

int hash_table_insert_copy(hash_table_t *table, const void *key, size_t key_size, const void *value, size_t value_size) {
    if (table == NULL || key == NULL || value == NULL) return 1;

    // Allocate memory and copy the value
    void *value_copy = malloc(value_size);
    if (value_copy == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for value copy.\n");
        return 1;
    }
    memcpy(value_copy, value, value_size);

    // Use regular insert with the copied value
    int result = hash_table_insert(table, key, key_size, value_copy);
    
    // If insert failed, free the copy we just made
    if (result != 0) {
        free(value_copy);
    }
    
    return result;
}

// FNV-1a hash function for arbitrary byte arrays
static size_t hash_table_fnv_1a(const void *key, size_t key_size) {
    const size_t fnv_prime = 0x1000193;
    size_t hash = 0x811C9DC5;
    const unsigned char *bytes = (const unsigned char *)key;

    for (size_t i = 0; i < key_size; i++) {
        hash ^= bytes[i];
        hash *= fnv_prime;
    }

    return hash;
}

// ============================================================================
// String Key Convenience Functions
// ============================================================================

int hash_table_insert_string(hash_table_t *table, const char *key, void *value) {
    if (key == NULL) return 1;
    return hash_table_insert(table, key, strlen(key) + 1, value);
}

int hash_table_insert_copy_string(hash_table_t *table, const char *key, const void *value, size_t value_size) {
    if (key == NULL) return 1;
    return hash_table_insert_copy(table, key, strlen(key) + 1, value, value_size);
}

void *hash_table_get_string(hash_table_t *table, const char *key) {
    if (key == NULL) return NULL;
    return hash_table_get(table, key, strlen(key) + 1);
}

void hash_table_remove_string(hash_table_t *table, const char *key) {
    if (key == NULL) return;
    hash_table_remove(table, key, strlen(key) + 1);
}

char hash_table_contains_string(hash_table_t *table, const char *key) {
    if (key == NULL) return 0;
    return hash_table_contains(table, key, strlen(key) + 1);
}
