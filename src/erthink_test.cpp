#include "test.h"
#include "erthink/erthink_d2a.h"

void dtoa_erthink(double v, char* buffer)
{
	*erthink::d2a(v, buffer) = '\0';
}

REGISTER_TEST(erthink);
