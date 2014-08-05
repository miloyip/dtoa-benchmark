#include "test.h"
#include "double-conversion/double-conversion.h"

using namespace double_conversion;

void dtoa_doubleconv(double value, char* buffer) {
	StringBuilder sb(buffer, 25);
	DoubleToStringConverter::EcmaScriptConverter().ToShortest(value, &sb);
}

REGISTER_TEST(doubleconv);
