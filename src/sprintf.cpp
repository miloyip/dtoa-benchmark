#include "test.h"
#include <cstdio>

void dtoa_sprintf(double value, char *buffer) {
  sprintf(buffer, "%.17g", value);
}

REGISTER_TEST(sprintf);
