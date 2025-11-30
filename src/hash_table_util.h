#pragma once

#include <stddef.h>

// Portable replacement for strdup (not part of ISO C)
char *ht_strdup(const char *s);
