#include <cstdio>
#include "test.h"

void dtoa_sprintf(double value, char* buffer) {
	sprintf(buffer, "%.17g", value);
}

REGISTER_TEST(sprintf);
