#include "hash_table.h"
#include "hash_table_util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

// Helper macro for string keys (includes null terminator)
#define SKEY(str) (str), strlen(str) + 1

// Example 1: Simple struct (trivially copyable)
typedef struct {
    int x;
    int y;
} point_t;

// Example 2: Custom type requiring special cleanup
typedef struct {
    char *name;
    int *data;
    size_t data_size;
} custom_data_t;

// Custom destructor for custom_data_t
void custom_data_destructor(void *value) {
    custom_data_t *data = (custom_data_t *)value;
    free(data->name);
    free(data->data);
    free(data);
}

// Example 3: Complex nested structure
typedef struct {
    char **strings;
    size_t count;
} string_array_t;

// Custom destructor for string_array_t
void string_array_destructor(void *value) {
    string_array_t *arr = (string_array_t *)value;
    for (size_t i = 0; i < arr->count; i++) {
        free(arr->strings[i]);
    }
    free(arr->strings);
    free(arr);
}

int main() {
    printf("=== Hash Table Examples ===\n\n");

    // ========================================
    // Example 1: Table with simple types (using insert_copy)
    // ========================================
    printf("Example 1: Simple types with insert_copy\n");
    printf("-----------------------------------------\n");
    
    hash_table_t *numbers_table = hash_table_create();
    if (numbers_table == NULL) {
        fprintf(stderr, "Failed to create numbers table\n");
        return 1;
    }

    // Insert primitive types using insert_copy (using string convenience functions)
    int age = 25;
    hash_table_insert_copy_string(numbers_table, "age", &age, sizeof(int));
    
    int score = 95;
    hash_table_insert_copy_string(numbers_table, "score", &score, sizeof(int));
    
    int level = 10;
    hash_table_insert_copy_string(numbers_table, "level", &level, sizeof(int));

    // Retrieve and print (using string convenience functions)
    int *retrieved_age = (int *)hash_table_get_string(numbers_table, "age");
    int *retrieved_score = (int *)hash_table_get_string(numbers_table, "score");
    int *retrieved_level = (int *)hash_table_get_string(numbers_table, "level");
    
    printf("  age: %d\n", retrieved_age ? *retrieved_age : -1);
    printf("  score: %d\n", retrieved_score ? *retrieved_score : -1);
    printf("  level: %d\n\n", retrieved_level ? *retrieved_level : -1);

    // ========================================
    // Example 2: Table with simple structs (using insert_copy)
    // ========================================
    printf("Example 2: Simple structs with insert_copy\n");
    printf("------------------------------------------\n");
    
    hash_table_t *points_table = hash_table_create();
    if (points_table == NULL) {
        fprintf(stderr, "Failed to create points table\n");
        hash_table_destroy(numbers_table);
        return 1;
    }

    // Insert structs using insert_copy (using string convenience functions)
    point_t origin = {0, 0};
    hash_table_insert_copy_string(points_table, "origin", &origin, sizeof(point_t));
    
    point_t center = {50, 50};
    hash_table_insert_copy_string(points_table, "center", &center, sizeof(point_t));

    // Retrieve and print (using string convenience functions)
    point_t *retrieved_origin = (point_t *)hash_table_get_string(points_table, "origin");
    point_t *retrieved_center = (point_t *)hash_table_get_string(points_table, "center");
    
    printf("  origin: (%d, %d)\n", retrieved_origin ? retrieved_origin->x : -1, 
           retrieved_origin ? retrieved_origin->y : -1);
    printf("  center: (%d, %d)\n\n", retrieved_center ? retrieved_center->x : -1,
           retrieved_center ? retrieved_center->y : -1);

    // ========================================
    // Example 3: Table with custom types (using custom destructor)
    // ========================================
    printf("Example 3: Custom types with custom destructor\n");
    printf("----------------------------------------------\n");
    
    hash_table_t *custom_table = hash_table_create_with_destructor(custom_data_destructor);
    if (custom_table == NULL) {
        fprintf(stderr, "Failed to create custom table\n");
        hash_table_destroy(numbers_table);
        hash_table_destroy(points_table);
        return 1;
    }

    // Create and insert custom data with heap-allocated fields
    custom_data_t *data1 = malloc(sizeof(custom_data_t));
    data1->name = ht_strdup("Dataset Alpha");
    data1->data_size = 3;
    data1->data = malloc(data1->data_size * sizeof(int));
    data1->data[0] = 10;
    data1->data[1] = 20;
    data1->data[2] = 30;
    hash_table_insert_string(custom_table, "dataset1", data1);

    custom_data_t *data2 = malloc(sizeof(custom_data_t));
    data2->name = ht_strdup("Dataset Beta");
    data2->data_size = 2;
    data2->data = malloc(data2->data_size * sizeof(int));
    data2->data[0] = 100;
    data2->data[1] = 200;
    hash_table_insert_string(custom_table, "dataset2", data2);

    // Retrieve and print (using string convenience functions)
    custom_data_t *retrieved1 = (custom_data_t *)hash_table_get_string(custom_table, "dataset1");
    if (retrieved1 != NULL) {
        printf("  %s: [", retrieved1->name);
        for (size_t i = 0; i < retrieved1->data_size; i++) {
            printf("%d%s", retrieved1->data[i], i < retrieved1->data_size - 1 ? ", " : "");
        }
        printf("]\n");
    }

    custom_data_t *retrieved2 = (custom_data_t *)hash_table_get_string(custom_table, "dataset2");
    if (retrieved2 != NULL) {
        printf("  %s: [", retrieved2->name);
        for (size_t i = 0; i < retrieved2->data_size; i++) {
            printf("%d%s", retrieved2->data[i], i < retrieved2->data_size - 1 ? ", " : "");
        }
        printf("]\n\n");
    }

    // ========================================
    // Example 4: Table with complex nested structures
    // ========================================
    printf("Example 4: Complex nested structures\n");
    printf("------------------------------------\n");
    
    hash_table_t *array_table = hash_table_create_with_destructor(string_array_destructor);
    if (array_table == NULL) {
        fprintf(stderr, "Failed to create array table\n");
        hash_table_destroy(numbers_table);
        hash_table_destroy(points_table);
        hash_table_destroy(custom_table);
        return 1;
    }

    // Create nested string arrays
    string_array_t *colors = malloc(sizeof(string_array_t));
    colors->count = 3;
    colors->strings = malloc(colors->count * sizeof(char *));
    colors->strings[0] = ht_strdup("red");
    colors->strings[1] = ht_strdup("green");
    colors->strings[2] = ht_strdup("blue");
    hash_table_insert_string(array_table, "colors", colors);

    string_array_t *fruits = malloc(sizeof(string_array_t));
    fruits->count = 4;
    fruits->strings = malloc(fruits->count * sizeof(char *));
    fruits->strings[0] = ht_strdup("apple");
    fruits->strings[1] = ht_strdup("banana");
    fruits->strings[2] = ht_strdup("orange");
    fruits->strings[3] = ht_strdup("grape");
    hash_table_insert_string(array_table, "fruits", fruits);

    // Retrieve and print (using string convenience functions)
    string_array_t *retrieved_colors = (string_array_t *)hash_table_get_string(array_table, "colors");
    if (retrieved_colors != NULL) {
        printf("  colors: [");
        for (size_t i = 0; i < retrieved_colors->count; i++) {
            printf("%s%s", retrieved_colors->strings[i], 
                   i < retrieved_colors->count - 1 ? ", " : "");
        }
        printf("]\n");
    }

    string_array_t *retrieved_fruits = (string_array_t *)hash_table_get_string(array_table, "fruits");
    if (retrieved_fruits != NULL) {
        printf("  fruits: [");
        for (size_t i = 0; i < retrieved_fruits->count; i++) {
            printf("%s%s", retrieved_fruits->strings[i], 
                   i < retrieved_fruits->count - 1 ? ", " : "");
        }
        printf("]\n\n");
    }

    // ========================================
    // Example 5: Integer keys (generic key feature)
    // ========================================
    printf("Example 5: Integer keys (generic key support)\n");
    printf("----------------------------------------------\n");
    
    hash_table_t *int_key_table = hash_table_create();
    if (int_key_table == NULL) {
        fprintf(stderr, "Failed to create int key table\n");
        hash_table_destroy(numbers_table);
        hash_table_destroy(points_table);
        hash_table_destroy(custom_table);
        hash_table_destroy(array_table);
        return 1;
    }

    // Use integers as keys
    int user_id_1 = 12345;
    int user_id_2 = 67890;
    int user_id_3 = 11111;
    
    char *name1 = ht_strdup("Alice");
    char *name2 = ht_strdup("Bob");
    char *name3 = ht_strdup("Charlie");
    
    hash_table_insert(int_key_table, &user_id_1, sizeof(int), name1);
    hash_table_insert(int_key_table, &user_id_2, sizeof(int), name2);
    hash_table_insert(int_key_table, &user_id_3, sizeof(int), name3);
    
    // Lookup by integer key
    int lookup_id = 67890;
    char *found_name = (char *)hash_table_get(int_key_table, &lookup_id, sizeof(int));
    printf("  User ID %d: %s\n", lookup_id, found_name ? found_name : "Not found");
    
    lookup_id = 12345;
    found_name = (char *)hash_table_get(int_key_table, &lookup_id, sizeof(int));
    printf("  User ID %d: %s\n\n", lookup_id, found_name ? found_name : "Not found");

    // ========================================
    // Example 6: Struct keys (generic key feature)
    // ========================================
    printf("Example 6: Struct keys (generic key support)\n");
    printf("--------------------------------------------\n");
    
    hash_table_t *struct_key_table = hash_table_create();
    if (struct_key_table == NULL) {
        fprintf(stderr, "Failed to create struct key table\n");
        hash_table_destroy(numbers_table);
        hash_table_destroy(points_table);
        hash_table_destroy(custom_table);
        hash_table_destroy(array_table);
        hash_table_destroy(int_key_table);
        return 1;
    }

    // Use point_t structs as keys to store color names
    point_t coord1 = {0, 0};
    point_t coord2 = {10, 20};
    point_t coord3 = {-5, 15};
    
    char *color1 = ht_strdup("Red");
    char *color2 = ht_strdup("Green");
    char *color3 = ht_strdup("Blue");
    
    hash_table_insert(struct_key_table, &coord1, sizeof(point_t), color1);
    hash_table_insert(struct_key_table, &coord2, sizeof(point_t), color2);
    hash_table_insert(struct_key_table, &coord3, sizeof(point_t), color3);
    
    // Lookup by struct key
    point_t lookup_coord = {10, 20};
    char *found_color = (char *)hash_table_get(struct_key_table, &lookup_coord, sizeof(point_t));
    printf("  Point (%d, %d): %s\n", lookup_coord.x, lookup_coord.y, 
           found_color ? found_color : "Not found");
    
    lookup_coord = (point_t){0, 0};
    found_color = (char *)hash_table_get(struct_key_table, &lookup_coord, sizeof(point_t));
    printf("  Point (%d, %d): %s\n\n", lookup_coord.x, lookup_coord.y,
           found_color ? found_color : "Not found");

    // ========================================
    // Clean up all tables
    // ========================================
    printf("Cleaning up all hash tables...\n");
    hash_table_destroy(numbers_table);
    hash_table_destroy(points_table);
    hash_table_destroy(custom_table);  // Will call custom_data_destructor for each entry
    hash_table_destroy(array_table);    // Will call string_array_destructor for each entry
    hash_table_destroy(int_key_table);
    hash_table_destroy(struct_key_table);
    
    printf("All examples completed successfully!\n");
    return 0;
}
