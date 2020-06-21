#define FMT_HEADER_ONLY 1
#include "fmt/format.h"
#include "test.h"

char *dtoa_fmt(double value, char *const buffer) {
  *fmt::format_to(buffer, "{}", value) = '\0';
  return buffer;
}

REGISTER_TEST(fmt);
