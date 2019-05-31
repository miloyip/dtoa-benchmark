#include "test.h"
#include <iomanip>
#include <sstream>

void dtoa_ostringstream(double value, char *buffer) {
  std::ostringstream oss;
  oss << std::setprecision(17) << value;
  strcpy(buffer, oss.str().c_str());
}

//#if RUN_CPPDTOA
REGISTER_TEST(ostringstream);
//#endif
