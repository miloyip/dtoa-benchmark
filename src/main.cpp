#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#if _MSC_VER
#include "msinttypes/stdint.h"
#else
#include <stdint.h>
#endif
#include "double-conversion/double-conversion.h"
#include "resultfilename.h"
#include "test.h"
#include "timer.h"
#include <cstdio>
#include <cstdlib>

const unsigned kVerifyRandomCount = 100000;
const unsigned kIterations = 10000;
const unsigned kRandomCount = 2000;
const unsigned kTrial = 42;
const int kMaxDigits = 17;

class Random {
public:
  Random(unsigned seed = 0) : mSeed(seed) {}

  uint64_t operator()() {
    mSeed =
        UINT64_C(6364136223846793005) * mSeed + UINT64_C(1442695040888963407);
    return mSeed;
  }

private:
  uint64_t mSeed;
};

static size_t VerifyValue(double value, char *(*f)(double, char *)) {
  char buffer[1024];
  char *str = f(value, buffer);

  // double-conversion returns correct result.
#if 0
  char *end = nullptr;
  const double roundtrip = strtod(str, &end);
  const size_t processed = end ? end - str : strnlen(str, sizeof(str));
#else
  using namespace double_conversion;
  StringToDoubleConverter converter(
      StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
  int processed = 0;
  double roundtrip = converter.StringToDouble(str, 1024, &processed);
#endif

  size_t len = strlen(str);
  if (len != (size_t)processed) {
    printf("Warning: some extra character %g -> '%s'\n", value, str);
    throw std::exception();
  }
  if (value != roundtrip) {
    printf(": roundtrip fail %.17g -> '%s' -> %.17g\n", value, str, roundtrip);
    throw std::exception();
  }

  return len;
}

static void Verify(const Case *it) {
  printf("Verifying %s...", it->fname);
  fflush(nullptr);

  // Boundary and simple cases
  VerifyValue(0, it->dtoa);
  VerifyValue(0.1, it->dtoa);
  VerifyValue(0.12, it->dtoa);
  VerifyValue(0.123, it->dtoa);
  VerifyValue(0.1234, it->dtoa);
  VerifyValue(1.2345, it->dtoa);
  VerifyValue(1.0 / 3.0, it->dtoa);
  VerifyValue(2.0 / 3.0, it->dtoa);
  VerifyValue(10.0 / 3.0, it->dtoa);
  VerifyValue(20.0 / 3.0, it->dtoa);
  VerifyValue(std::numeric_limits<double>::min(), it->dtoa);
  VerifyValue(std::numeric_limits<double>::max(), it->dtoa);
  VerifyValue(std::numeric_limits<double>::denorm_min(), it->dtoa);

  Random rnd;
  union {
    double d;
    uint64_t u;
  } u;

  u.u = UINT64_C(0x345E0FFED391517E);
  VerifyValue(u.d, it->dtoa);
  u.u = UINT64_C(0xF6EA6C767640CD71);
  VerifyValue(u.d, it->dtoa);

  uint64_t lenSum = 0;
  size_t lenMax = 0;
  for (unsigned i = 0; i < kVerifyRandomCount; i++) {
    do
      u.u = rnd();
    while (isnan(u.d) || isinf(u.d));
    size_t len = VerifyValue(u.d, it->dtoa);
    lenSum += len;
    lenMax = std::max(lenMax, len);
  }

  double lenAvg = double(lenSum) / kVerifyRandomCount;
  printf(" OK. Length Avg = %2.3f, Max = %d\n", lenAvg, (int)lenMax);
  fflush(nullptr);
}

void VerifyAll() {
  const TestList &tests = TestManager::Instance().GetTests();

  for (TestList::const_iterator itr = tests.begin(); itr != tests.end();
       ++itr) {
    if (strcmp((*itr)->fname, "null") != 0) { // skip null
      try {
        Verify(*itr);
      } catch (...) {
      }
    }
  }
}

//------------------------------------------------------------------------------

void BenchSequential(Case *it, FILE *fp) {
  printf("Benchmarking  sequential %s...", it->fname);
  fflush(nullptr);

  Random rnd;
  char buffer[256];
  it->reset();

  int64_t start = 1;
  for (int digit = 1; digit <= kMaxDigits; digit++) {
    int64_t end = start * 10;

    double duration = std::numeric_limits<double>::max();
    for (unsigned trial = 0; trial < kTrial; trial++) {
      int64_t v = start + int64_t(rnd()) % start;
      double sign = 1;
      Timer timer;
      timer.Start();
      for (unsigned iteration = 0; iteration < kIterations; iteration++) {
        double d = v * sign;
        it->dtoa(d, buffer);
        // printf("%.17g -> %s\n", d, buffer);
        sign = -sign;
        v += 1;
        if (v >= end)
          v = start;
      }
      timer.Stop();
      duration = std::min(duration, timer.GetElapsedMilliseconds());
    }

    duration *= 1e6 / kIterations; // convert to nano second per operation
    it->account(duration);
    fprintf(fp, "sequential,%s,%d,%f\n", it->fname, digit, duration);
    start = end;
  }
  printf(" Done\n");
  fflush(nullptr);
}

//------------------------------------------------------------------------------

class RandomData {
public:
  static double *GetData() {
    static RandomData singleton;
    return singleton.mData;
  }

private:
  RandomData() : mData(new double[kRandomCount]) {
    Random rnd;
    union {
      double d;
      uint64_t u;
    } u;

    for (size_t i = 0; i < kRandomCount; i++) {
      do
        u.u = rnd();
      while (isnan(u.d) || isinf(u.d));
      mData[i] = u.d;
    }
  }

  ~RandomData() { delete[] mData; }

  double *mData;
};

void BenchRandom(Case *it, FILE *fp) {
  printf("Benchmarking      random %s...", it->fname);
  fflush(nullptr);

  char buffer[256];
  it->reset();
  double *data = RandomData::GetData();

  double duration = std::numeric_limits<double>::max();
  for (unsigned trial = 0; trial < kTrial; trial++) {
    Timer timer;
    timer.Start();

    static_assert(kIterations % kRandomCount == 0, "Oops");
    for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
         iteration++)
      for (size_t i = 0; i < kRandomCount; i++)
        it->dtoa(data[i], buffer);

    timer.Stop();
    duration = std::min(duration, timer.GetElapsedMilliseconds());
  }

  duration *= 1e6 / kIterations; // convert to nano second per operation
  it->account(duration);
  fprintf(fp, "random,%s,0,%f\n", it->fname, duration);

  printf(" Done\n");
  fflush(nullptr);
}

//------------------------------------------------------------------------------

class RandomDigitData {
public:
  static double *GetData(int digit) {
    assert(digit >= 1 && digit <= kMaxDigits);
    static RandomDigitData singleton;
    return singleton.mData + (digit - 1) * kRandomCount;
  }

private:
  RandomDigitData() : mData(new double[kMaxDigits * kRandomCount]) {
    Random rnd;
    union {
      double d;
      uint64_t u;
    } u;

    double *p = mData;
    for (int digit = 1; digit <= kMaxDigits; digit++) {
      for (size_t i = 0; i < kRandomCount; i++) {
        do
          u.u = rnd();
        while (isnan(u.d) || isinf(u.d));

        // Convert to string with limited digits, and convert it back.
        char buffer[256];
        sprintf(buffer, "%.*g", digit, u.d);
#if 0
        char *end = nullptr;
        const double roundtrip =strtod(buffer, &end);
        const size_t processed = end ? end - buffer : strnlen(buffer, sizeof(buffer));
#else
        using namespace double_conversion;
        StringToDoubleConverter converter(
            StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
        int processed = 0;
        double roundtrip = converter.StringToDouble(buffer, 256, &processed);
#endif

        *p++ = roundtrip;
      }
    }
  }

  ~RandomDigitData() { delete[] mData; }

  double *mData;
};

void BenchRandomDigit(Case *it, FILE *fp) {
  printf("Benchmarking randomdigit %s...", it->fname);
  fflush(nullptr);

  char buffer[256];
  it->reset();

  for (int digit = 1; digit <= kMaxDigits; digit++) {
    double *data = RandomDigitData::GetData(digit);

    double duration = std::numeric_limits<double>::max();
    for (unsigned trial = 0; trial < kTrial; trial++) {
      Timer timer;
      timer.Start();

      static_assert(kIterations % kRandomCount == 0, "Oops");
      for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
           iteration++) {
        for (size_t i = 0; i < kRandomCount; i++) {
          it->dtoa(data[i], buffer);
          // if (trial == 0 && iteration == 0 && i == 0)
          //	printf("%.17g -> %s\n", data[i], buffer);
        }
      }

      timer.Stop();
      duration = std::min(duration, timer.GetElapsedMilliseconds());
    }

    duration *= 1e6 / kIterations; // convert to nano second per operation
    it->account(duration);
    fprintf(fp, "randomdigit,%s,%d,%f\n", it->fname, digit, duration);
  }
  printf(" Done\n");
  fflush(nullptr);
}

//------------------------------------------------------------------------------

void Bench(Case *it, FILE *fp) {
  // BenchSequential(it, fp);
  // BenchRandom(it, fp);
  BenchRandomDigit(it, fp);
}

void BenchAll() {
  // doublery to write to /result path, where template.php exists
  FILE *fp;
  if ((fp = fopen("../../result/template.php", "r")) != NULL) {
    fclose(fp);
    fp = fopen("../../result/" RESULT_FILENAME, "w");
  } else if ((fp = fopen("../result/template.php", "r")) != NULL) {
    fclose(fp);
    fp = fopen("../result/" RESULT_FILENAME, "w");
  } else
    fp = fopen(RESULT_FILENAME, "w");

  fprintf(fp, "Type,Function,Digit,Time(ns)\n");

  for (auto it : TestManager::Instance().GetTests())
    Bench(it, fp);

  fclose(fp);
}

void TestManager::Sort() {
  std::sort(mTests.begin(), mTests.end(),
            [](const Case *a, const Case *b) { return *a < *b; });
}

void TestManager::PrintScores(Score score, bool SkipWorseThanBaseline) const {
  double baseline = 0;
  for (const auto it : mTests)
    if (it->baseline) {
      baseline = it->*score;
      break;
    }

  bool single_column = true;
  std::vector<const Case *> vector;
  for (const auto it : mTests) {
    if (SkipWorseThanBaseline && baseline && baseline <= it->*score)
      continue;
    if (it->count > 1)
      single_column = false;
    vector.push_back(it);
  }

  std::sort(
      vector.begin(), vector.end(),
      [score](const Case *a, const Case *b) { return a->*score < b->*score; });

  printf("Function      %s|   Sum ns  | Speedup |\n",
         single_column ? "" : "|  Min ns |  RMS ns  |  Max ns ");
  printf(":-------------%s|----------:|--------:|\n",
         single_column ? "" : "|--------:|---------:|--------:");

  for (const auto it : vector) {
    printf("%-14s|", it->fname);
    if (!single_column)
      printf("%8.1f |%9.3f |%8.1f |", it->min, it->rms, it->max);
    printf("%10.1f | ×%-6.1f |\n", it->sum,
           (baseline ? baseline : 1.0) / it->*score);
  }
}

int main() {
  TestManager::Instance().Sort();
  VerifyAll();
  BenchAll();
  TestManager::Instance().PrintScores(&Case::sum);
}
