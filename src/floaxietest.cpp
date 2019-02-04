#include "test.h"
#include "floaxie/ftoa.h"

void dtoa_floaxie(double v, char* buffer)
{
	floaxie::ftoa(v, buffer);
}

REGISTER_TEST(floaxie);
