#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"
#include "test.h"

void dtoa_stb(double value, char *buffer) {
  stbsp_sprintf(buffer, "%.17g", value);
}

REGISTER_TEST(stb);
