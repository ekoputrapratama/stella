#include <cerrno>
#include <limits.h>
#include <stdlib.h>

// Heimdall
#include "Utility.h"
#include "flasher.h"

NumberParsingStatus Utility::ParseInt(int &intValue, const char *string, int base) {
  errno = 0;

  char *end;
  long longValue = strtol(string, &end, base);

  if (*string == '\0' || *end != '\0') {
    return (kNumberParsingStatusInconvertible);
  } else if (errno == ERANGE) {
    intValue = (longValue == LONG_MAX) ? INT_MAX : INT_MIN;
    return (kNumberParsingStatusRangeError);
  } else if (longValue > INT_MAX) {
    intValue = INT_MAX;
    return (kNumberParsingStatusRangeError);
  } else if (longValue < INT_MIN) {
    intValue = INT_MIN;
    return (kNumberParsingStatusRangeError);
  }

  intValue = longValue;
  return (kNumberParsingStatusSuccess);
}

NumberParsingStatus Utility::ParseUnsignedInt(unsigned int &uintValue, const char *string,
                                              int base) {
  errno = 0;

  char *end;
  unsigned long ulongValue = strtoul(string, &end, base);

  if (*string == '\0' || *end != '\0') {
    return kNumberParsingStatusInconvertible;
  } else if (errno == ERANGE || ulongValue > INT_MAX) {
    uintValue = UINT_MAX;
    return (kNumberParsingStatusRangeError);
  }

  uintValue = ulongValue;
  return (kNumberParsingStatusSuccess);
}
