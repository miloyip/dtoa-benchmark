#include "test.h"

extern "C" {
char *g_fmt(char *, double);
}

char *dtoa_gay(double value, char *const buffer) {
  g_fmt(buffer, value);
  return buffer;
}

#if 0 /* Disabled because of invalid result and/or SIGSEGV */
REGISTER_TEST(gay);
#endif
