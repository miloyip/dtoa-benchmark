#include "test.h"
#include <iomanip>
#include <strstream>

char *dtoa_ostrstream(double value, char *const buffer) {
  std::ostrstream oss(buffer, 25);
  oss << std::setprecision(17) << value << std::ends;
  return strcpy(buffer, oss.str());
}

//#if RUN_CPPITOA
REGISTER_TEST(ostrstream);
//#endif
