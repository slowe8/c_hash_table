#include "test_framework.h"
#include "../src/hash_table.h"
#include "../src/hash_table_util.h"
#include <stdlib.h>
#include <string.h>

// Helper macro for string keys (includes null terminator)
#define SKEY(str) (str), strlen(str) + 1

// ========================================
// Basic Operations Tests
// ========================================

TEST(test_create_and_destroy) {
    hash_table_t *table = hash_table_create();
    ASSERT_NOT_NULL(table, "Table should be created");
    ASSERT_EQ(0, hash_table_size(table), "New table should have size 0");
    ASSERT_EQ(HASH_TABLE_DEFAULT_INITIAL_CAPACITY, hash_table_capacity(table), 
              "New table should have default capacity");
    hash_table_destroy(table);
}

TEST(test_create_with_custom_capacity) {
    hash_table_t *table = hash_table_create_with_parameters(32, 0.75, 2.0);
    ASSERT_NOT_NULL(table, "Table should be created");
    ASSERT_EQ(32, hash_table_capacity(table), "Table should have custom capacity");
    hash_table_destroy(table);
}

TEST(test_insert_and_get_single) {
    hash_table_t *table = hash_table_create();
    
    int *value = malloc(sizeof(int));
    *value = 42;
    
    int result = hash_table_insert(table, SKEY("test_key"), value);
    ASSERT_EQ(0, result, "Insert should succeed");
    ASSERT_EQ(1, hash_table_size(table), "Size should be 1 after insert");
    
    int *retrieved = (int *)hash_table_get(table, SKEY("test_key"));
    ASSERT_NOT_NULL(retrieved, "Should retrieve inserted value");
    ASSERT_EQ(42, *retrieved, "Retrieved value should match inserted value");
    
    hash_table_destroy(table);
}

TEST(test_insert_copy_primitives) {
    hash_table_t *table = hash_table_create();
    
    int x = 100;
    int result = hash_table_insert_copy(table, SKEY("number"), &x, sizeof(int));
    ASSERT_EQ(0, result, "Insert copy should succeed");
    
    int *retrieved = (int *)hash_table_get(table, SKEY("number"));
    ASSERT_NOT_NULL(retrieved, "Should retrieve copied value");
    ASSERT_EQ(100, *retrieved, "Retrieved value should match");
    
    // Modify original - copy should be unaffected
    x = 200;
    ASSERT_EQ(100, *retrieved, "Copy should be independent of original");
    
    hash_table_destroy(table);
}

TEST(test_insert_multiple_values) {
    hash_table_t *table = hash_table_create();
    
    for (int i = 0; i < 10; i++) {
        int *value = malloc(sizeof(int));
        *value = i * 10;
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        hash_table_insert(table, key, strlen(key)+1, value);
    }
    
    ASSERT_EQ(10, hash_table_size(table), "Size should be 10");
    
    for (int i = 0; i < 10; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        int *retrieved = (int *)hash_table_get(table, key, strlen(key)+1);
        ASSERT_NOT_NULL(retrieved, "Should retrieve all values");
        ASSERT_EQ(i * 10, *retrieved, "Values should match");
    }
    
    hash_table_destroy(table);
}

TEST(test_update_existing_key) {
    hash_table_t *table = hash_table_create();
    
    int *value1 = malloc(sizeof(int));
    *value1 = 42;
    hash_table_insert(table, SKEY("key"), value1);
    
    int *value2 = malloc(sizeof(int));
    *value2 = 99;
    hash_table_insert(table, SKEY("key"), value2);
    
    ASSERT_EQ(1, hash_table_size(table), "Size should still be 1 after update");
    
    int *retrieved = (int *)hash_table_get(table, SKEY("key"));
    ASSERT_NOT_NULL(retrieved, "Should retrieve updated value");
    ASSERT_EQ(99, *retrieved, "Value should be updated");
    
    hash_table_destroy(table);
}

TEST(test_get_nonexistent_key) {
    hash_table_t *table = hash_table_create();
    
    void *result = hash_table_get(table, SKEY("nonexistent"));
    ASSERT_NULL(result, "Should return NULL for nonexistent key");
    
    hash_table_destroy(table);
}

TEST(test_contains) {
    hash_table_t *table = hash_table_create();
    
    int *value = malloc(sizeof(int));
    *value = 42;
    hash_table_insert(table, SKEY("exists"), value);
    
    ASSERT_EQ(1, hash_table_contains(table, SKEY("exists")), "Should contain inserted key");
    ASSERT_EQ(0, hash_table_contains(table, SKEY("missing")), "Should not contain missing key");
    
    hash_table_destroy(table);
}

// ========================================
// Remove Tests
// ========================================

TEST(test_remove_existing_key) {
    hash_table_t *table = hash_table_create();
    
    int *value = malloc(sizeof(int));
    *value = 42;
    hash_table_insert(table, SKEY("key"), value);
    
    ASSERT_EQ(1, hash_table_size(table), "Size should be 1");
    
    hash_table_remove(table, SKEY("key"));
    
    ASSERT_EQ(0, hash_table_size(table), "Size should be 0 after remove");
    ASSERT_NULL(hash_table_get(table, SKEY("key")), "Key should not exist after remove");
    
    hash_table_destroy(table);
}

TEST(test_remove_nonexistent_key) {
    hash_table_t *table = hash_table_create();
    
    // Should not crash
    hash_table_remove(table, SKEY("nonexistent"));
    ASSERT_EQ(0, hash_table_size(table), "Size should remain 0");
    
    hash_table_destroy(table);
}

TEST(test_remove_multiple) {
    hash_table_t *table = hash_table_create();
    
    for (int i = 0; i < 5; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        hash_table_insert(table, key, strlen(key)+1, value);
    }
    
    hash_table_remove(table, SKEY("key_2"));
    ASSERT_EQ(4, hash_table_size(table), "Size should be 4");
    ASSERT_NULL(hash_table_get(table, SKEY("key_2")), "Removed key should not exist");
    ASSERT_NOT_NULL(hash_table_get(table, SKEY("key_1")), "Other keys should still exist");
    ASSERT_NOT_NULL(hash_table_get(table, SKEY("key_3")), "Other keys should still exist");
    
    hash_table_destroy(table);
}

// ========================================
// Clear Tests
// ========================================

TEST(test_clear) {
    hash_table_t *table = hash_table_create();
    
    for (int i = 0; i < 10; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        hash_table_insert(table, key, strlen(key)+1, value);
    }
    
    ASSERT_EQ(10, hash_table_size(table), "Size should be 10");
    
    hash_table_clear(table);
    
    ASSERT_EQ(0, hash_table_size(table), "Size should be 0 after clear");
    ASSERT_NULL(hash_table_get(table, SKEY("key_0")), "Keys should not exist after clear");
    
    hash_table_destroy(table);
}

// ========================================
// Resize Tests
// ========================================

TEST(test_resize) {
    hash_table_t *table = hash_table_create_with_parameters(4, 0.75, 2.0);
    
    // Insert enough to trigger resize
    for (int i = 0; i < 10; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        hash_table_insert(table, key, strlen(key)+1, value);
    }
    
    ASSERT_EQ(10, hash_table_size(table), "All values should be inserted");
    
    // Verify all values still accessible after resize
    for (int i = 0; i < 10; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        int *retrieved = (int *)hash_table_get(table, key, strlen(key)+1);
        ASSERT_NOT_NULL(retrieved, "Should retrieve values after resize");
        ASSERT_EQ(i, *retrieved, "Values should be correct after resize");
    }
    
    hash_table_destroy(table);
}

// ========================================
// Edge Cases Tests
// ========================================

TEST(test_null_table_operations) {
    // Should not crash with NULL table
    ASSERT_EQ(0, hash_table_size(NULL), "Size of NULL table should be 0");
    ASSERT_EQ(0, hash_table_capacity(NULL), "Capacity of NULL table should be 0");
    ASSERT_NULL(hash_table_get(NULL, SKEY("key")), "Get on NULL table should return NULL");
    ASSERT_EQ(0, hash_table_contains(NULL, SKEY("key")), "Contains on NULL should return 0");
    ASSERT_EQ(1, hash_table_insert(NULL, SKEY("key"), NULL), "Insert on NULL should fail");
    hash_table_remove(NULL, SKEY("key"));  // Should not crash
    hash_table_clear(NULL);  // Should not crash
    hash_table_destroy(NULL);  // Should not crash
}

TEST(test_null_key_operations) {
    hash_table_t *table = hash_table_create();
    
    int *value = malloc(sizeof(int));
    *value = 42;
    
    ASSERT_EQ(1, hash_table_insert(table, NULL, 0, value), "Insert with NULL key should fail");
    ASSERT_NULL(hash_table_get(table, NULL, 0), "Get with NULL key should return NULL");
    ASSERT_EQ(0, hash_table_contains(table, NULL, 0), "Contains with NULL key should return 0");
    hash_table_remove(table, NULL, 0);  // Should not crash
    
    free(value);  // Free since insert failed
    hash_table_destroy(table);
}

TEST(test_empty_key) {
    hash_table_t *table = hash_table_create();
    
    int *value = malloc(sizeof(int));
    *value = 42;
    hash_table_insert(table, SKEY(""), value);
    
    int *retrieved = (int *)hash_table_get(table, SKEY(""));
    ASSERT_NOT_NULL(retrieved, "Empty string should be valid key");
    ASSERT_EQ(42, *retrieved, "Should retrieve value with empty key");
    
    hash_table_destroy(table);
}

TEST(test_long_keys) {
    hash_table_t *table = hash_table_create();
    
    char long_key[1000];
    memset(long_key, 'a', sizeof(long_key) - 1);
    long_key[sizeof(long_key) - 1] = '\0';
    
    int *value = malloc(sizeof(int));
    *value = 42;
    hash_table_insert(table, long_key, strlen(long_key)+1, value);
    
    int *retrieved = (int *)hash_table_get(table, long_key, strlen(long_key)+1);
    ASSERT_NOT_NULL(retrieved, "Should handle long keys");
    ASSERT_EQ(42, *retrieved, "Should retrieve value with long key");
    
    hash_table_destroy(table);
}

TEST(test_collision_handling) {
    hash_table_t *table = hash_table_create_with_parameters(4, 0.99, 2.0);
    
    // Insert multiple values to force collisions
    for (int i = 0; i < 20; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        char key[32];
        snprintf(key, sizeof(key), "collision_key_%d", i);
        hash_table_insert(table, key, strlen(key)+1, value);
    }
    
    // Verify all values are accessible despite collisions
    for (int i = 0; i < 20; i++) {
        char key[32];
        snprintf(key, sizeof(key), "collision_key_%d", i);
        int *retrieved = (int *)hash_table_get(table, key, strlen(key)+1);
        ASSERT_NOT_NULL(retrieved, "Should handle collisions");
        ASSERT_EQ(i, *retrieved, "Values should be correct with collisions");
    }
    
    hash_table_destroy(table);
}

// ========================================
// Custom Destructor Tests
// ========================================

typedef struct {
    int *data;
    char *name;
} custom_type_t;

static int destructor_call_count = 0;

void custom_destructor(void *value) {
    custom_type_t *obj = (custom_type_t *)value;
    free(obj->data);
    free(obj->name);
    free(obj);
    destructor_call_count++;
}

TEST(test_custom_destructor) {
    destructor_call_count = 0;
    
    hash_table_t *table = hash_table_create_with_destructor(custom_destructor);
    
    for (int i = 0; i < 5; i++) {
        custom_type_t *obj = malloc(sizeof(custom_type_t));
        obj->data = malloc(sizeof(int));
        *obj->data = i;
        obj->name = ht_strdup("test");
        
        char key[32];
        snprintf(key, sizeof(key), "obj_%d", i);
        hash_table_insert(table, key, strlen(key)+1, obj);
    }
    
    hash_table_destroy(table);
    
    ASSERT_EQ(5, destructor_call_count, "Destructor should be called for each value");
}

TEST(test_custom_destructor_on_update) {
    destructor_call_count = 0;
    
    hash_table_t *table = hash_table_create_with_destructor(custom_destructor);
    
    custom_type_t *obj1 = malloc(sizeof(custom_type_t));
    obj1->data = malloc(sizeof(int));
    *obj1->data = 1;
    obj1->name = ht_strdup("first");
    hash_table_insert(table, SKEY("key"), obj1);
    
    custom_type_t *obj2 = malloc(sizeof(custom_type_t));
    obj2->data = malloc(sizeof(int));
    *obj2->data = 2;
    obj2->name = ht_strdup("second");
    hash_table_insert(table, SKEY("key"), obj2);
    
    ASSERT_EQ(1, destructor_call_count, "Destructor should be called on update");
    
    hash_table_destroy(table);
    
    ASSERT_EQ(2, destructor_call_count, "Destructor should be called on destroy");
}

TEST(test_custom_destructor_on_remove) {
    destructor_call_count = 0;
    
    hash_table_t *table = hash_table_create_with_destructor(custom_destructor);
    
    custom_type_t *obj = malloc(sizeof(custom_type_t));
    obj->data = malloc(sizeof(int));
    *obj->data = 42;
    obj->name = ht_strdup("test");
    hash_table_insert(table, SKEY("key"), obj);
    
    hash_table_remove(table, SKEY("key"));
    
    ASSERT_EQ(1, destructor_call_count, "Destructor should be called on remove");
    
    hash_table_destroy(table);
}

// ========================================
// Generic Key Tests
// ========================================

TEST(test_integer_keys) {
    hash_table_t *table = hash_table_create();
    
    // Use integers as keys
    int key1 = 100;
    int key2 = 200;
    int key3 = 300;
    
    int *val1 = malloc(sizeof(int));
    *val1 = 1000;
    int *val2 = malloc(sizeof(int));
    *val2 = 2000;
    int *val3 = malloc(sizeof(int));
    *val3 = 3000;
    
    hash_table_insert(table, &key1, sizeof(int), val1);
    hash_table_insert(table, &key2, sizeof(int), val2);
    hash_table_insert(table, &key3, sizeof(int), val3);
    
    ASSERT_EQ(3, hash_table_size(table), "Size should be 3");
    
    // Retrieve with integer keys
    int lookup = 200;
    int *retrieved = (int *)hash_table_get(table, &lookup, sizeof(int));
    ASSERT_NOT_NULL(retrieved, "Should retrieve with int key");
    ASSERT_EQ(2000, *retrieved, "Value should match");
    
    // Test contains with int key
    ASSERT_EQ(1, hash_table_contains(table, &key1, sizeof(int)), "Should contain key");
    
    int missing = 999;
    ASSERT_EQ(0, hash_table_contains(table, &missing, sizeof(int)), "Should not contain missing key");
    
    hash_table_destroy(table);
}

typedef struct {
    int x;
    int y;
} test_point_t;

TEST(test_struct_keys) {
    hash_table_t *table = hash_table_create();
    
    // Use structs as keys
    test_point_t p1 = {0, 0};
    test_point_t p2 = {10, 20};
    test_point_t p3 = {-5, 15};
    
    int *val1 = malloc(sizeof(int));
    *val1 = 111;
    int *val2 = malloc(sizeof(int));
    *val2 = 222;
    int *val3 = malloc(sizeof(int));
    *val3 = 333;
    
    hash_table_insert(table, &p1, sizeof(test_point_t), val1);
    hash_table_insert(table, &p2, sizeof(test_point_t), val2);
    hash_table_insert(table, &p3, sizeof(test_point_t), val3);
    
    ASSERT_EQ(3, hash_table_size(table), "Size should be 3");
    
    // Retrieve with struct key
    test_point_t lookup = {10, 20};
    int *retrieved = (int *)hash_table_get(table, &lookup, sizeof(test_point_t));
    ASSERT_NOT_NULL(retrieved, "Should retrieve with struct key");
    ASSERT_EQ(222, *retrieved, "Value should match");
    
    // Remove with struct key
    hash_table_remove(table, &p1, sizeof(test_point_t));
    ASSERT_EQ(2, hash_table_size(table), "Size should be 2 after remove");
    ASSERT_NULL(hash_table_get(table, &p1, sizeof(test_point_t)), "Removed key should not exist");
    
    hash_table_destroy(table);
}

TEST(test_mixed_key_sizes) {
    hash_table_t *table = hash_table_create();
    
    // Insert keys of different sizes
    char small_key = 'A';
    short medium_key = 1000;
    long long large_key = 999999999LL;
    
    int *val1 = malloc(sizeof(int));
    *val1 = 1;
    int *val2 = malloc(sizeof(int));
    *val2 = 2;
    int *val3 = malloc(sizeof(int));
    *val3 = 3;
    
    hash_table_insert(table, &small_key, sizeof(char), val1);
    hash_table_insert(table, &medium_key, sizeof(short), val2);
    hash_table_insert(table, &large_key, sizeof(long long), val3);
    
    ASSERT_EQ(3, hash_table_size(table), "Should handle different key sizes");
    
    // Retrieve all
    int *r1 = (int *)hash_table_get(table, &small_key, sizeof(char));
    int *r2 = (int *)hash_table_get(table, &medium_key, sizeof(short));
    int *r3 = (int *)hash_table_get(table, &large_key, sizeof(long long));
    
    ASSERT_NOT_NULL(r1, "Should retrieve char key");
    ASSERT_NOT_NULL(r2, "Should retrieve short key");
    ASSERT_NOT_NULL(r3, "Should retrieve long long key");
    
    ASSERT_EQ(1, *r1, "Char key value should match");
    ASSERT_EQ(2, *r2, "Short key value should match");
    ASSERT_EQ(3, *r3, "Long long key value should match");
    
    hash_table_destroy(table);
}

// ========================================
// Stress Tests
// ========================================

TEST(test_large_dataset) {
    hash_table_t *table = hash_table_create();
    
    const int count = 10000;
    
    // Insert many values
    for (int i = 0; i < count; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        hash_table_insert(table, key, strlen(key)+1, value);
    }
    
    ASSERT_EQ(count, hash_table_size(table), "Should handle large dataset");
    
    // Verify random access
    int *val = (int *)hash_table_get(table, SKEY("key_500"));
    ASSERT_NOT_NULL(val, "Should retrieve from large dataset");
    ASSERT_EQ(500, *val, "Value should be correct");
    
    hash_table_destroy(table);
}

// ========================================
// Main Test Runner
// ========================================

int main(int argc, char *argv[]) {
    // Check for test filter
    test_filter = getenv("TEST_FILTER");
    
    if (test_filter != NULL) {
        printf("========================================\n");
        printf("Running filtered test: %s\n", test_filter);
        printf("========================================\n\n");
    } else {
        printf("========================================\n");
        printf("Hash Table Test Suite\n");
        printf("========================================\n\n");
    }
    
    (void)argc;
    (void)argv;
    
    printf("Basic Operations Tests:\n");
    RUN_TEST(test_create_and_destroy);
    RUN_TEST(test_create_with_custom_capacity);
    RUN_TEST(test_insert_and_get_single);
    RUN_TEST(test_insert_copy_primitives);
    RUN_TEST(test_insert_multiple_values);
    RUN_TEST(test_update_existing_key);
    RUN_TEST(test_get_nonexistent_key);
    RUN_TEST(test_contains);
    
    printf("\nRemove Tests:\n");
    RUN_TEST(test_remove_existing_key);
    RUN_TEST(test_remove_nonexistent_key);
    RUN_TEST(test_remove_multiple);
    
    printf("\nClear Tests:\n");
    RUN_TEST(test_clear);
    
    printf("\nResize Tests:\n");
    RUN_TEST(test_resize);
    
    printf("\nEdge Cases Tests:\n");
    RUN_TEST(test_null_table_operations);
    RUN_TEST(test_null_key_operations);
    RUN_TEST(test_empty_key);
    RUN_TEST(test_long_keys);
    RUN_TEST(test_collision_handling);
    
    printf("\nCustom Destructor Tests:\n");
    RUN_TEST(test_custom_destructor);
    RUN_TEST(test_custom_destructor_on_update);
    RUN_TEST(test_custom_destructor_on_remove);
    
    printf("\nGeneric Key Tests:\n");
    RUN_TEST(test_integer_keys);
    RUN_TEST(test_struct_keys);
    RUN_TEST(test_mixed_key_sizes);
    
    printf("\nStress Tests:\n");
    RUN_TEST(test_large_dataset);
    
    TEST_SUMMARY();
}
