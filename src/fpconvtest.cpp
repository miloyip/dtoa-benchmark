#include "test.h"

extern "C" {
#include "fpconv/fpconv.h"
}

void dtoa_fpconv(double value, char *buffer) {
  buffer[fpconv_dtoa(value, buffer)] = '\0';
}

REGISTER_TEST(fpconv);
