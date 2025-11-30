/*
 * Performance benchmarks for hash_table
 * 
 * Compiled with optimizations enabled (no sanitizers) to measure
 * real-world performance characteristics.
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "hash_table.h"
#include "hash_table_util.h"

// SKEY macro for string keys
#define SKEY(str) (str), strlen(str) + 1

// Timing utilities
typedef struct {
    struct timespec start;
    struct timespec end;
} bench_timer_t;

static void timer_start(bench_timer_t *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

static double timer_end(bench_timer_t *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
    double start_ns = timer->start.tv_sec * 1e9 + timer->start.tv_nsec;
    double end_ns = timer->end.tv_sec * 1e9 + timer->end.tv_nsec;
    return (end_ns - start_ns) / 1e9; // Return seconds
}

// Generate random string key
static char *generate_key(int index, char *buffer, size_t buf_size) {
    snprintf(buffer, buf_size, "key_%d_%u", index, (unsigned)(index * 2654435761u));
    return buffer;
}

// Benchmark: Insert N string keys with integer values
static void bench_insert_strings(size_t n) {
    printf("\n=== Insert %zu String Keys ===\n", n);
    
    hash_table_t *table = hash_table_create();
    bench_timer_t timer;
    char key_buffer[64];
    
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", n / elapsed);
    printf("  Size:       %zu entries\n", hash_table_size(table));
    printf("  Capacity:   %zu slots\n", hash_table_capacity(table));
    printf("  Load:       %.2f%%\n", 100.0 * hash_table_size(table) / hash_table_capacity(table));
    
    hash_table_destroy(table);
}

// Benchmark: Insert N integer keys with integer values
static void bench_insert_integers(size_t n) {
    printf("\n=== Insert %zu Integer Keys ===\n", n);
    
    hash_table_t *table = hash_table_create();
    bench_timer_t timer;
    
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        int key = (int)i;
        int *value = malloc(sizeof(int));
        *value = (int)(i * 2);
        hash_table_insert(table, &key, sizeof(int), value);
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", n / elapsed);
    printf("  Size:       %zu entries\n", hash_table_size(table));
    printf("  Capacity:   %zu slots\n", hash_table_capacity(table));
    printf("  Load:       %.2f%%\n", 100.0 * hash_table_size(table) / hash_table_capacity(table));
    
    hash_table_destroy(table);
}

// Benchmark: Lookup N keys (successful lookups)
static void bench_lookup_strings(size_t n) {
    printf("\n=== Lookup %zu String Keys (All Found) ===\n", n);
    
    // Populate table
    hash_table_t *table = hash_table_create();
    char key_buffer[64];
    
    for (size_t i = 0; i < n; i++) {
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
    }
    
    // Benchmark lookups
    bench_timer_t timer;
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        int *value = (int *)hash_table_get_string(table, key_buffer);
        if (!value || *value != (int)i) {
            fprintf(stderr, "ERROR: Lookup failed for key %zu\n", i);
            exit(1);
        }
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", n / elapsed);
    printf("  Avg lookup: %.3f microseconds\n", (elapsed / n) * 1e6);
    
    hash_table_destroy(table);
}

// Benchmark: Lookup with mixed hits and misses
static void bench_lookup_mixed(size_t n) {
    printf("\n=== Lookup %zu Keys (50%% Hit Rate) ===\n", n * 2);
    
    // Populate table with n entries
    hash_table_t *table = hash_table_create();
    char key_buffer[64];
    
    for (size_t i = 0; i < n; i++) {
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
    }
    
    // Lookup n existing keys and n non-existing keys
    bench_timer_t timer;
    size_t hits = 0;
    
    timer_start(&timer);
    for (size_t i = 0; i < n * 2; i++) {
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        void *value = hash_table_get_string(table, key_buffer);
        if (value) hits++;
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", (n * 2) / elapsed);
    printf("  Hit rate:   %.1f%% (%zu hits)\n", 100.0 * hits / (n * 2), hits);
    printf("  Avg lookup: %.3f microseconds\n", (elapsed / (n * 2)) * 1e6);
    
    hash_table_destroy(table);
}

// Benchmark: Remove N keys
static void bench_remove_strings(size_t n) {
    printf("\n=== Remove %zu String Keys ===\n", n);
    
    // Populate table
    hash_table_t *table = hash_table_create();
    char key_buffer[64];
    
    for (size_t i = 0; i < n; i++) {
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
    }
    
    // Benchmark removals
    bench_timer_t timer;
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_remove_string(table, key_buffer);
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", n / elapsed);
    printf("  Final size: %zu entries\n", hash_table_size(table));
    
    hash_table_destroy(table);
}

// Benchmark: Insert and lookup interleaved
static void bench_mixed_workload(size_t n) {
    printf("\n=== Mixed Workload (%zu inserts + %zu lookups) ===\n", n, n);
    
    hash_table_t *table = hash_table_create();
    char key_buffer[64];
    bench_timer_t timer;
    
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        // Insert
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
        
        // Lookup (every other insert, lookup a previous key)
        if (i > 0 && i % 2 == 0) {
            generate_key((int)(i / 2), key_buffer, sizeof(key_buffer));
            hash_table_get_string(table, key_buffer);
        }
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", (n * 1.5) / elapsed);
    printf("  Final size: %zu entries\n", hash_table_size(table));
    
    hash_table_destroy(table);
}

// Benchmark: Contains checks
static void bench_contains(size_t n) {
    printf("\n=== Contains Check %zu Keys ===\n", n);
    
    // Populate table
    hash_table_t *table = hash_table_create();
    char key_buffer[64];
    
    for (size_t i = 0; i < n; i++) {
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
    }
    
    // Benchmark contains
    bench_timer_t timer;
    size_t found = 0;
    
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        if (hash_table_contains_string(table, key_buffer)) {
            found++;
        }
    }
    double elapsed = timer_end(&timer);
    
    printf("  Time:       %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f ops/sec\n", n / elapsed);
    printf("  Found:      %zu/%zu keys\n", found, n);
    
    hash_table_destroy(table);
}

// Benchmark: Struct keys (coordinates)
static void bench_struct_keys(size_t n) {
    printf("\n=== Insert/Lookup %zu Struct Keys (Point) ===\n", n);
    
    typedef struct {
        int x;
        int y;
    } Point;
    
    hash_table_t *table = hash_table_create();
    bench_timer_t timer;
    
    // Insert
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        Point key = {(int)(i % 1000), (int)(i / 1000)};
        int *value = malloc(sizeof(int));
        *value = (int)i;
        hash_table_insert(table, &key, sizeof(Point), value);
    }
    double insert_time = timer_end(&timer);
    
    // Lookup
    size_t hits = 0;
    timer_start(&timer);
    for (size_t i = 0; i < n; i++) {
        Point key = {(int)(i % 1000), (int)(i / 1000)};
        void *value = hash_table_get(table, &key, sizeof(Point));
        if (value) hits++;
    }
    double lookup_time = timer_end(&timer);
    
    printf("  Insert time: %.6f sec (%.0f ops/sec)\n", insert_time, n / insert_time);
    printf("  Lookup time: %.6f sec (%.0f ops/sec)\n", lookup_time, n / lookup_time);
    printf("  Hit rate:    100%% (%zu hits)\n", hits);
    
    hash_table_destroy(table);
}

// Memory efficiency test
static void bench_memory_efficiency(size_t n) {
    printf("\n=== Memory Efficiency (%zu entries) ===\n", n);
    
    hash_table_t *table = hash_table_create();
    char key_buffer[64];
    
    for (size_t i = 0; i < n; i++) {
        int *value = malloc(sizeof(int));
        *value = (int)i;
        generate_key((int)i, key_buffer, sizeof(key_buffer));
        hash_table_insert_string(table, key_buffer, value);
    }
    
    size_t capacity = hash_table_capacity(table);
    size_t size = hash_table_size(table);
    size_t wasted = capacity - size;
    
    // Rough memory calculation
    size_t key_mem = size * 30; // Average key length estimate
    size_t value_mem = size * sizeof(int);
    size_t overhead = capacity * (sizeof(void*) * 2 + sizeof(size_t) + 1); // keys, values, key_sizes, occupied
    size_t total_mem = key_mem + value_mem + overhead;
    
    printf("  Entries:    %zu\n", size);
    printf("  Capacity:   %zu slots\n", capacity);
    printf("  Load:       %.2f%%\n", 100.0 * size / capacity);
    printf("  Wasted:     %zu slots (%.2f%%)\n", wasted, 100.0 * wasted / capacity);
    printf("  Est. mem:   %.2f KB\n", total_mem / 1024.0);
    printf("  Per entry:  %.0f bytes\n", (double)total_mem / size);
    
    hash_table_destroy(table);
}

int main(void) {
    printf("========================================\n");
    printf("Hash Table Performance Benchmarks\n");
    printf("========================================\n");
    printf("Compiled with optimizations enabled\n");
    printf("No sanitizers, no debug checks\n");
    
    // Run benchmarks at different scales - including larger and irregular sizes
    size_t sizes[] = {
        1000,      // 1K
        5000,      // 5K
        10000,     // 10K
        25000,     // 25K
        50000,     // 50K
        100000,    // 100K
        250000,    // 250K
        500000,    // 500K
        1000000    // 1M
    };
    
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        size_t n = sizes[i];
        
        printf("\n\n");
        printf("========================================\n");
        printf("SCALE: %zu entries\n", n);
        printf("========================================\n");
        
        bench_insert_strings(n);
        bench_insert_integers(n);
        bench_lookup_strings(n);
        bench_lookup_mixed(n / 2);
        bench_remove_strings(n);
        bench_contains(n);
        bench_mixed_workload(n);
        bench_struct_keys(n);
        bench_memory_efficiency(n);
    }
    
    printf("\n\n");
    printf("========================================\n");
    printf("All benchmarks completed successfully!\n");
    printf("========================================\n");
    
    return 0;
}
