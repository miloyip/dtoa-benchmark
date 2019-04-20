#include "test.h"
#include "double-conversion/double-conversion.h"

using namespace double_conversion;

void dtoa_doubleconv(double value, char* buffer) {
	StringBuilder sb(buffer, 26);
	DoubleToStringConverter::EcmaScriptConverter().ToShortest(value, &sb);
	sb.Finalize();
}

REGISTER_TEST(doubleconv);
