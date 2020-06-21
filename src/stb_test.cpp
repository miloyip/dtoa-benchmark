#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"
#include "test.h"

char *dtoa_stb(double value, char *const buffer) {
  stbsp_sprintf(buffer, "%.17g", value);
  return buffer;
}

REGISTER_TEST(stb);
