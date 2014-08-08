#include "test.h"

extern "C" {
char *g_fmt(char *, double);
}

void dtoa_gay(double value, char* buffer) {
	g_fmt(buffer, value);
}

//REGISTER_TEST(gay);
