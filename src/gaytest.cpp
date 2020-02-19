#include "test.h"

extern "C" {
char *g_fmt(char *, double);
}

void dtoa_gay(double value, char *buffer) { g_fmt(buffer, value); }

#if 0 /* Disabled because of invalid result and/or SIGSEGV */
REGISTER_TEST(gay);
#endif
