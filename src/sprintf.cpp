#include "test.h"
#include <cstdio>

void dtoa_sprintf(double value, char *buffer) {
  sprintf(buffer, "%.17g", value);
}

static Case gRegister_sprintf("sprintf", dtoa_sprintf, true);
