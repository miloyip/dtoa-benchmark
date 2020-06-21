#include "test.h"
#include <cstdio>

char *dtoa_sprintf(double value, char *const buffer) {
  sprintf(buffer, "%.17g", value);
  return buffer;
}

static Case gRegister_sprintf("sprintf", dtoa_sprintf, true);
