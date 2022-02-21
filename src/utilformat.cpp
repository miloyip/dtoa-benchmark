#include "test.h"
#include "util/format.h"

void dtoa_util(double value, char* buffer) {
  buffer = util::format_to(buffer, "{:.17}",  value);
  *buffer = '\0';
}

REGISTER_TEST(util);
