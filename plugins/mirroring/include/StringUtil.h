#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#include <stddef.h>

// return the index to truncate a UTF-8 string at a valid position
size_t utf8_truncation_index(const char *utf8, size_t max_len);
#endif
