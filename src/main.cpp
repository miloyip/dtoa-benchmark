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
#include <cstdlib>
#include <math.h>
#include "resultfilename.h"
#include "timer.h"
#include "test.h"
#include "double-conversion/double-conversion.h"

const unsigned kVerifyRandomCount = 100000;
const unsigned kIterationForRandom = 100;
const unsigned kIterationPerDigit = 10;
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

static size_t VerifyValue(double value, void(*f)(double, char*)) {
	char buffer[1024];
	f(value, buffer);

	// double-conversion returns correct result.
	using namespace double_conversion;
	StringToDoubleConverter converter(StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
	int processed = 0;
	double roundtrip = converter.StringToDouble(buffer, 1024, &processed);

	size_t len = strlen(buffer);
	if (len != (size_t)processed) {
		printf("Error: some extra character %g -> '%s'\n", value, buffer);
		throw std::exception();
	}
	if (value != roundtrip) {
		printf("Error: roundtrip fail %.17g -> '%s' -> %.17g\n", value, buffer, roundtrip);
		//throw std::exception();
	}

	return len;
}

static void Verify(void(*f)(double, char*), const char* fname) {
	printf("Verifying %-20s ... ", fname);

	// Boundary and simple cases
	VerifyValue(0, f);
	VerifyValue(0.1, f);
	VerifyValue(0.12, f);
	VerifyValue(0.123, f);
	VerifyValue(0.1234, f);
	VerifyValue(1.2345, f);
	VerifyValue(1.0 / 3.0, f);
	VerifyValue(2.0 / 3.0, f);
	VerifyValue(10.0 / 3.0, f);
	VerifyValue(20.0 / 3.0, f);
	VerifyValue(std::numeric_limits<double>::min(), f);
	VerifyValue(std::numeric_limits<double>::max(), f);
	VerifyValue(std::numeric_limits<double>::denorm_min(), f);
	
	union {
		double d;
		uint64_t u;
	}u;
	Random r;

	uint64_t lenSum = 0;
	size_t lenMax = 0;
	for (unsigned i = 0; i < kVerifyRandomCount; i++) {
		do {
			// Need to call r() in two statements for cross-platform coherent sequence.
			u.u = uint64_t(r()) << 32;
			u.u |= uint64_t(r());
		} while (isnan(u.d) || isinf(u.d));
		size_t len = VerifyValue(u.d, f);
		lenSum += len;
		lenMax = std::max(lenMax, len);
	}

	double lenAvg = double(lenSum) / kVerifyRandomCount;
	printf("OK. Length Avg = %2.3f, Max = %d\n", lenAvg, (int)lenMax);
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

void BenchSequential(void(*f)(double, char*), const char* fname, FILE* fp) {
	printf("Benchmarking  sequential %-20s ... ", fname);

	char buffer[256] = { '\0' };
	double minDuration = std::numeric_limits<double>::max();
	double maxDuration = 0.0;
        double rmsDuration = 0.0;
        int N = 0;

	int64_t start = 1;
	for (int digit = 1; digit <= 17; digit++) {
		int64_t end = start * 10;

		double duration = std::numeric_limits<double>::max();
		for (unsigned trial = 0; trial < kTrial; trial++) {
			int64_t v = start;
			Random r;
			v += ((int64_t(r()) << 32) | int64_t(r())) % start;
			double sign = 1;
			Timer timer;
			timer.Start();
			for (unsigned iteration = 0; iteration < kIterationPerDigit; iteration++) {
				double d = v * sign;
				f(d, buffer);
				//printf("%.17g -> %s\n", d, buffer);
				sign = -sign;
				v += 1;
				if (v >= end)
					v = start;
			}
			timer.Stop();
			duration = std::min(duration, timer.GetElapsedMilliseconds());
		}

		duration *= 1e6 / kIterationPerDigit; // convert to nano second per operation
		minDuration = std::min(minDuration, duration);
		maxDuration = std::max(maxDuration, duration);
                rmsDuration += duration * duration;
                N += 1;
		fprintf(fp, "%s_sequential,%d,%f\n", fname, digit, duration);
		start = end;
	}

	printf("[min %8.3fns, rms %8.3fns, max %8.3fns]\n", minDuration, sqrt(rmsDuration/N), maxDuration);
}

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

		for (size_t i = 0; i < kCount; i++) {
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
	printf("Benchmarking      random %-20s ... ", fname);

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

	printf("%8.3fns\n", duration);
}

class RandomDigitData {
public:
	static double* GetData(int digit) {
		assert(digit >= 1 && digit <= 17);
		static RandomDigitData singleton;
		return singleton.mData + (digit - 1) * kCount;
	}

	static const int kMaxDigit = 17;
	static const size_t kCount = 1000;

private:
	RandomDigitData() :
		mData(new double[kMaxDigit * kCount])
	{
		Random r;
		union {
			double d;
			uint64_t u;
		}u;

		double* p = mData;
		for (int digit = 1; digit <= kMaxDigit; digit++) {
			for (size_t i = 0; i < kCount; i++) {
				do {
					// Need to call r() in two statements for cross-platform coherent sequence.
					u.u = uint64_t(r()) << 32;
					u.u |= uint64_t(r());
				} while (isnan(u.d) || isinf(u.d));

				// Convert to string with limited digits, and convert it back.
				char buffer[256];
				sprintf(buffer, "%.*g", digit, u.d);
				using namespace double_conversion;
				StringToDoubleConverter converter(StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
				int processed = 0;
				double roundtrip = converter.StringToDouble(buffer, 256, &processed);

				*p++ = roundtrip;
			}
		}
	}

	~RandomDigitData() {
		delete[] mData;
	}

	double* mData;
};

void BenchRandomDigit(void(*f)(double, char*), const char* fname, FILE* fp) {
	printf("Benchmarking randomdigit %-20s ... ", fname);

	char buffer[256];
	double minDuration = std::numeric_limits<double>::max();
	double maxDuration = 0.0;
        double rmsDuration = 0.0;
        int N = 0;

	for (int digit = 1; digit <= RandomDigitData::kMaxDigit; digit++) {
		double* data = RandomDigitData::GetData(digit);
		size_t n = RandomDigitData::kCount;

		double duration = std::numeric_limits<double>::max();
		for (unsigned trial = 0; trial < kTrial; trial++) {
			Timer timer;
			timer.Start();

			for (unsigned iteration = 0; iteration < kIterationPerDigit; iteration++) {
				for (size_t i = 0; i < n; i++) {
					f(data[i], buffer);
					//if (trial == 0 && iteration == 0 && i == 0)
					//	printf("%.17g -> %s\n", data[i], buffer);
				}
			}

			timer.Stop();
			duration = std::min(duration, timer.GetElapsedMilliseconds());
		}

		duration *= 1e6 / (kIterationPerDigit * n); // convert to nano second per operation
		minDuration = std::min(minDuration, duration);
		maxDuration = std::max(maxDuration, duration);
                rmsDuration += duration * duration;
                N += 1;
		fprintf(fp, "randomdigit,%s,%d,%f\n", fname, digit, duration);
	}
        printf("[min %8.3fns, rms %8.3fns, max %8.3fns]\n", minDuration, sqrt(rmsDuration/N), maxDuration);
}

void Bench(void(*f)(double, char*), const char* fname, FILE* fp) {
	//BenchSequential(f, fname, fp);
	//BenchRandom(f, fname, fp);
	BenchRandomDigit(f, fname, fp);
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

	fprintf(fp, "Type,Function,Digit,Time(ns)\n");

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
