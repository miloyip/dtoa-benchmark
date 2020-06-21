#include "test.h"
#include <iomanip>
#include <sstream>

char *dtoa_ostringstream(double value, char *const buffer) {
  std::ostringstream oss;
  oss << std::setprecision(17) << value;
  return strcpy(buffer, oss.str().c_str());
}

//#if RUN_CPPDTOA
REGISTER_TEST(ostringstream);
//#endif
