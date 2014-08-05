#include "test.h"

extern "C" {
bool fill_double(double v, char* buffer);
}

void dtoa_grisu2(double value, char* buffer) {
	if (value == 0)
		strcpy(buffer, "0.0");
	else {
		if (value < 0) {
			*buffer++ = '-';
			value = -value;
		}
		fill_double(value, buffer);
	}
}

REGISTER_TEST(grisu2);
