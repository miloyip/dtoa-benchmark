#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf/stb_sprintf.h"
#include "test.h"

void dtoa_stb_sprintf(double value, char *buffer) {
  stbsp_sprintf(buffer, "%.17g", value);
}

REGISTER_TEST(stb_sprintf);
