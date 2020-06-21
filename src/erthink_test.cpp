#include "erthink/erthink_d2a.h"
#include "test.h"

char *dtoa_erthink(double v, char *const buffer) {
  *erthink::d2a_fast(v, buffer) = '\0';
  return buffer;
}

REGISTER_TEST(erthink);
