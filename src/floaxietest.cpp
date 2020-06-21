#include "floaxie/ftoa.h"
#include "test.h"

char *dtoa_floaxie(double v, char *const buffer) {
  floaxie::ftoa(v, buffer);
  return buffer;
}

REGISTER_TEST(floaxie);
