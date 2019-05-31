#include "test.h"
#include "src/ryu/ryu.h"

void dtoa_ryu(double v, char *buffer) {
  const int n = d2s_buffered_n(v, buffer);
  buffer[n] = '\0';
}

REGISTER_TEST(ryu);
