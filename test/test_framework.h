#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Test framework macros
static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;
static const char *test_filter = NULL;

#define TEST(name) \
    static void name(void); \
    static void run_##name(void) { \
        printf("  Running: %s... ", #name); \
        fflush(stdout); \
        name(); \
        printf("PASSED\n"); \
        test_passed++; \
    } \
    static void name(void)

#define RUN_TEST(test) \
    do { \
        if (test_filter == NULL || strcmp(test_filter, #test) == 0) { \
            test_count++; \
            run_##test(); \
        } \
    } while(0)

#define RUN_TEST_WITH_CLEANUP(test, cleanup) \
    do { \
        test_count++; \
        run_##test(); \
        cleanup(); \
    } while(0)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAILED\n"); \
            printf("    Assertion failed: %s\n", message); \
            printf("    at %s:%d\n", __FILE__, __LINE__); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    ASSERT((expected) == (actual), message)

#define ASSERT_NE(expected, actual, message) \
    ASSERT((expected) != (actual), message)

#define ASSERT_NULL(ptr, message) \
    ASSERT((ptr) == NULL, message)

#define ASSERT_NOT_NULL(ptr, message) \
    ASSERT((ptr) != NULL, message)

#define ASSERT_STR_EQ(expected, actual, message) \
    ASSERT(strcmp((expected), (actual)) == 0, message)

#define TEST_SUMMARY() \
    do { \
        printf("\n========================================\n"); \
        printf("Test Summary:\n"); \
        printf("  Total:  %d\n", test_count); \
        printf("  Passed: %d\n", test_passed); \
        printf("  Failed: %d\n", test_failed); \
        printf("========================================\n"); \
        if (test_failed > 0) { \
            printf("TESTS FAILED\n"); \
            return 1; \
        } else { \
            printf("ALL TESTS PASSED\n"); \
            return 0; \
        } \
    } while(0)
