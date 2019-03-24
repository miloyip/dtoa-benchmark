#define FMT_HEADER_ONLY 1
#include "test.h"
#include "fmt/format.h"

void dtoa_fmt(double value, char* buffer) {
  buffer = fmt::format_to(buffer, "{}", value);
  *buffer = '\0';
}

REGISTER_TEST(fmt);
