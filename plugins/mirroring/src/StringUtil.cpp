#include "StringUtil.h"

#include <string.h>

size_t utf8_truncation_index(const char *utf8, size_t max_len) {
  size_t len = strlen(utf8);
  if (len <= max_len) {
    return len;
  }
  len = max_len;
  // see UTF-8 encoding <https://en.wikipedia.org/wiki/UTF-8#Description>
  while ((utf8[len] & 0x80) != 0 && (utf8[len] & 0xc0) != 0xc0) {
    // the next byte is not the start of a new UTF-8 codepoint
    // so if we would cut there, the character would be truncated
    len--;
  }
  return len;
}
