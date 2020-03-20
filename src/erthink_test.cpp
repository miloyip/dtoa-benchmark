#include "erthink/erthink_d2a.h"
#include "test.h"

void dtoa_erthink(double v, char *buffer) {
  *erthink::d2a_fast(v, buffer) = '\0';
}

REGISTER_TEST(erthink);
