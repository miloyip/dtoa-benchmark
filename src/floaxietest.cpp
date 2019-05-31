#include "floaxie/ftoa.h"
#include "test.h"

void dtoa_floaxie(double v, char *buffer) { floaxie::ftoa(v, buffer); }

REGISTER_TEST(floaxie);
