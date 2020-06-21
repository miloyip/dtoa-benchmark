#include "test.h"

extern "C" {
bool fill_double(double v, char *buffer);
}

char *dtoa_grisu2(double value, char *const buffer) {
  if (value == 0)
    strcpy(buffer, "0.0");
  else {
    char *str = buffer;
    if (value < 0) {
      *str++ = '-';
      value = -value;
    }
    fill_double(value, str);
  }
  return buffer;
}

REGISTER_TEST(grisu2);
