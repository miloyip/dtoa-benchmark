#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#if _MSC_VER
#include "msinttypes/stdint.h"
#else
#include <stdint.h>
#endif
#include <stdlib.h>
#include "resultfilename.h"
#include "timer.h"
#include "test.h"
#include "double-conversion/double-conversion.h"

const unsigned kIterationForRandom = 100;
const unsigned kTrial = 10;

class Random {
public:
	Random(unsigned seed = 0) : mSeed(seed) {}

	unsigned operator()() {
		mSeed = 214013 * mSeed + 2531011;
		return mSeed;
	}

private:
	unsigned mSeed;
};

static void VerifyValue(double value, void(*f)(double, char*)) {
	char buffer[1024];
	f(value, buffer);

	//printf("%.17g -> %s\n", value, buffer);

#if 0
	char* end;
	double roundtrip = strtod(buffer, &end);
	int processed = int(end - buffer);
#else
	// double-conversion returns correct result.
	using namespace double_conversion;
	StringToDoubleConverter converter(StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
	int processed = 0;
	double roundtrip = converter.StringToDouble(buffer, 1024, &processed);
#endif

	if (strlen(buffer) != (size_t)processed) {
		printf("Error: some extra character %g -> '%s'\n", value, buffer);
		throw std::exception();
	}
	if (value != roundtrip) {
		printf("Error: roundtrip fail %.17g -> '%s' -> %.17g\n", value, buffer, roundtrip);
		//throw std::exception();
	}
}

static void Verify(void(*f)(double, char*), const char* fname) {
	printf("Verifying %s ... ", fname);

	// Boundary and simple cases
	VerifyValue(0, f);
	VerifyValue(0.1, f);
	VerifyValue(0.12, f);
	VerifyValue(0.123, f);
	VerifyValue(0.1234, f);
	VerifyValue(1.0 / 3.0, f);
	VerifyValue(2.0 / 3.0, f);
	VerifyValue(10.0 / 3.0, f);
	VerifyValue(20.0 / 3.0, f);
	VerifyValue(std::numeric_limits<double>::min(), f);
	VerifyValue(std::numeric_limits<double>::max(), f);
	
	Random r;
	union {
		double d;
		uint64_t u;
	}u;

	for (int i = 0; i < 100000; i++) {
		do {
			// Need to call r() in two statements for cross-platform coherent sequence.
			u.u = uint64_t(r()) << 32;
			u.u |= uint64_t(r());
		} while (isnan(u.d) || isinf(u.d));
		VerifyValue(u.d, f);
	}

	printf("OK\n");
}

void VerifyAll() {
	const TestList& tests = TestManager::Instance().GetTests();

	for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr) {
		if (strcmp((*itr)->fname, "null") != 0) {	// skip null
			try {
				Verify((*itr)->dtoa, (*itr)->fname);
			}
			catch (...) {
			}
		}
	}
}

//template <typename double>
//void BenchSequential(void(*f)(double, char*), const char* type, const char* fname, FILE* fp) {
//	printf("Benchmarking sequential %-20s ... ", fname);
//
//	char buffer[doubleraits<double>::kBufferSize];
//	double minDuration = std::numeric_limits<double>::max();
//	double maxDuration = 0.0;
//
//	double start = 1;
//	for (int digit = 1; digit <= doubleraits<double>::kMaxDigit; digit++) {
//		double end = (digit == doubleraits<double>::kMaxDigit) ? std::numeric_limits<double>::max() : start * 10;
//
//		double duration = std::numeric_limits<double>::max();
//		for (unsigned trial = 0; trial < kTrial; trial++) {
//			double v = start;
//			double sign = 1;
//			Timer timer;
//			timer.Start();
//			for (unsigned iteration = 0; iteration < kIterationPerDigit; iteration++) {
//				f(v * sign, buffer);
//				sign = doubleraits<double>::Negate(sign);
//				if (++v == end)
//					v = start;
//			}
//			timer.Stop();
//			duration = std::min(duration, timer.GetElapsedMilliseconds());
//		}
//
//		minDuration = std::min(minDuration, duration);
//		maxDuration = std::max(maxDuration, duration);
//		fprintf(fp, "%s_sequential,%s,%d,%f\n", type, fname, digit, duration);
//		start = end;
//	}
//
//	printf("[%8.3fms, %8.3fms]\n", minDuration, maxDuration);
//}
//

class RandomData {
public:
	static double* GetData() {
		static RandomData singleton;
		return singleton.mData;
	}

	static const size_t kCount = 1000;

private:
	RandomData() :
		mData(new double[kCount])
	{
		Random r;
		union {
			double d;
			uint64_t u;
		}u;

		for (int i = 0; i < kCount; i++) {
			do {
				// Need to call r() in two statements for cross-platform coherent sequence.
				u.u = uint64_t(r()) << 32;
				u.u |= uint64_t(r());
			} while (isnan(u.d) || isinf(u.d));
			mData[i] = u.d;
		}
	}

	~RandomData() {
		delete[] mData;
	}

	double* mData;
};

void BenchRandom(void(*f)(double, char*), const char* fname, FILE* fp) {
	printf("Benchmarking     random %-20s ... ", fname);

	char buffer[256];
	double* data = RandomData::GetData();
	size_t n = RandomData::kCount;

	double duration = std::numeric_limits<double>::max();
	for (unsigned trial = 0; trial < kTrial; trial++) {
		Timer timer;
		timer.Start();

		for (unsigned iteration = 0; iteration < kIterationForRandom; iteration++)
			for (size_t i = 0; i < n; i++)
				f(data[i], buffer);

		timer.Stop();
		duration = std::min(duration, timer.GetElapsedMilliseconds());
	}

	duration *= 1e6 / (kIterationForRandom * n); // convert to nano second per operation

	fprintf(fp, "random,%s,0,%f\n", fname, duration);

	printf("%8.3fns per op\n", duration);
}

void Bench(void(*f)(double, char*), const char* fname, FILE* fp) {
	//BenchSequential(f, type, fname, fp);
	BenchRandom(f, fname, fp);
}


void BenchAll() {
	// doublery to write to /result path, where template.php exists
	FILE *fp;
	if ((fp = fopen("../../result/template.php", "r")) != NULL) {
		fclose(fp);
		fp = fopen("../../result/" RESULT_FILENAME, "w");
	}
	else if ((fp = fopen("../result/template.php", "r")) != NULL) {
		fclose(fp);
		fp = fopen("../result/" RESULT_FILENAME, "w");
	}
	else
		fp = fopen(RESULT_FILENAME, "w");

	fprintf(fp, "doubleype,Function,Digit,doubleime(ms)\n");

	const TestList& tests = TestManager::Instance().GetTests();

	for (TestList::const_iterator itr = tests.begin(); itr != tests.end(); ++itr)
		Bench((*itr)->dtoa, (*itr)->fname, fp);

	fclose(fp);
}

int main() {
	// sort tests
	TestList& tests = TestManager::Instance().GetTests();
	std::sort(tests.begin(), tests.end());

	VerifyAll();
	BenchAll();
}
