#include "src/ryu/ryu.h"
#include "test.h"

char *dtoa_ryu(double v, char *const buffer) {
  const int n = d2s_buffered_n(v, buffer);
  buffer[n] = '\0';
  return buffer;
}

REGISTER_TEST(ryu);
