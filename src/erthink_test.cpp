#include "erthink/erthink_d2a.h"
#include "test.h"

char *dtoa_erthink(double v, char *const buffer) {
  *erthink::d2a_fast(v, buffer) = '\0';
  return buffer;
}

REGISTER_TEST(erthink);

char *dtoa_erthink_shodan(double v, char *const buffer) {
  erthink::grisu::shodan_printer<true> printer(
      buffer, buffer + erthink::grisu::shodan_printer<>::buffer_size);
  erthink::grisu::convert(printer, v);
  const auto begin_end = printer.finalize_and_get();
  *begin_end.second = '\0';
  return begin_end.first;
}

REGISTER_TEST(erthink_shodan);
