#include "test.h"

extern "C" {
#include "fpconv/fpconv.h"
}

char *dtoa_fpconv(double value, char *const buffer) {
  buffer[fpconv_dtoa(value, buffer)] = '\0';
  return buffer;
}

REGISTER_TEST(fpconv);
