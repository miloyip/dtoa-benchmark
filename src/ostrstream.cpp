#include <iomanip>
#include <strstream>
#include "test.h"

void dtoa_ostrstream(double value, char* buffer) {
	std::ostrstream oss(buffer, 25);
	oss << std::setprecision(17) << value << std::ends;
	strcpy(buffer, oss.str());
}

//#if RUN_CPPITOA
REGISTER_TEST(ostrstream);
//#endif
