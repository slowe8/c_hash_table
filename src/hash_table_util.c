#include "hash_table_util.h"

#include <stdlib.h>
#include <string.h>

char *ht_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}
