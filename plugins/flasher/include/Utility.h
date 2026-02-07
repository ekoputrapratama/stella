#ifndef UTILITY_H
#define UTILITY_H

typedef enum {
  kNumberParsingStatusSuccess = 0,
  kNumberParsingStatusRangeError,
  kNumberParsingStatusInconvertible
} NumberParsingStatus;

namespace Utility {
  NumberParsingStatus ParseInt(int &intValue, const char *string, int base = 0);
  NumberParsingStatus ParseUnsignedInt(unsigned int &uintValue, const char *string, int base = 0);
}

#endif
