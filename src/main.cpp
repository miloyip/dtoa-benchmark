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

static void VerifyValue(double value, void(*f)(double, char*), const char* fname) {
	char buffer[1024];
	//printf("%.17g\n", value);

	f(value, buffer);

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
	VerifyValue(0, f, fname);
	VerifyValue(1.0 / 3.0, f, fname);
	VerifyValue(2.0 / 3.0, f, fname);
	VerifyValue(10.0 / 3.0, f, fname);
	VerifyValue(20.0 / 3.0, f, fname);
	VerifyValue(std::numeric_limits<double>::min(), f, fname);
	VerifyValue(std::numeric_limits<double>::max(), f, fname);

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
		VerifyValue(u.d, f, fname);
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

//template <typename T>
//void BenchSequential(void(*f)(T, char*), const char* type, const char* fname, FILE* fp) {
//	printf("Benchmarking sequential %-20s ... ", fname);
//
//	char buffer[Traits<T>::kBufferSize];
//	double minDuration = std::numeric_limits<double>::max();
//	double maxDuration = 0.0;
//
//	T start = 1;
//	for (int digit = 1; digit <= Traits<T>::kMaxDigit; digit++) {
//		T end = (digit == Traits<T>::kMaxDigit) ? std::numeric_limits<T>::max() : start * 10;
//
//		double duration = std::numeric_limits<double>::max();
//		for (unsigned trial = 0; trial < kTrial; trial++) {
//			T v = start;
//			T sign = 1;
//			Timer timer;
//			timer.Start();
//			for (unsigned iteration = 0; iteration < kIterationPerDigit; iteration++) {
//				f(v * sign, buffer);
//				sign = Traits<T>::Negate(sign);
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
//template <class T>
//class RandomData {
//public:
//	static T* GetData() {
//		static RandomData singleton;
//		return singleton.mData;
//	}
//
//	static const size_t kCountPerDigit = 1000;
//	static const size_t kCount = kCountPerDigit * Traits<T>::kMaxDigit;
//
//private:
//	RandomData() :
//		mData(new T[kCount])
//	{
//		T* p = mData;
//		T start = 1;
//		for (int digit = 1; digit <= Traits<T>::kMaxDigit; digit++) {
//			T end = (digit == Traits<T>::kMaxDigit) ? std::numeric_limits<T>::max() : start * 10;
//			T v = start;
//			T sign = 1;
//			for (size_t i = 0; i < kCountPerDigit; i++) {
//				*p++ = v * sign;
//				sign = Traits<T>::Negate(sign);
//				if (++v == end)
//					v = start;
//			}
//			start = end;
//		}
//		std::random_shuffle(mData, mData + kCount);
//	}
//
//	~RandomData() {
//		delete[] mData;
//	}
//
//	T* mData;
//};
//
//template <typename T>
//void BenchRandom(void(*f)(T, char*), const char* type, const char* fname, FILE* fp) {
//	printf("Benchmarking     random %-20s ... ", fname);
//
//	char buffer[Traits<T>::kBufferSize];
//	T* data = RandomData<T>::GetData();
//	size_t n = RandomData<T>::kCount;
//
//	double duration = std::numeric_limits<double>::max();
//	for (unsigned trial = 0; trial < kTrial; trial++) {
//		Timer timer;
//		timer.Start();
//
//		for (unsigned iteration = 0; iteration < kIterationForRandom; iteration++)
//		for (size_t i = 0; i < n; i++)
//			f(data[i], buffer);
//
//		timer.Stop();
//		duration = std::min(duration, timer.GetElapsedMilliseconds());
//	}
//	fprintf(fp, "%s_random,%s,0,%f\n", type, fname, duration);
//
//	printf("%8.3fms\n", duration);
//}
//
void Bench(void(*f)(double, char*), const char* fname, FILE* fp) {
	//BenchSequential(f, type, fname, fp);
	//BenchRandom(f, type, fname, fp);
}


void BenchAll() {
	// Try to write to /result path, where template.php exists
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

	fprintf(fp, "Type,Function,Digit,Time(ms)\n");

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
	//BenchAll();
}
