#pragma once

#include <limits>
#include <math.h>
#include <string.h>
#include <vector>

struct Case;
typedef std::vector<Case *> TestList;
typedef const double Case::*Score;

class TestManager {
public:
  static TestManager &Instance() {
    static TestManager singleton;
    return singleton;
  }

  void AddTest(Case *test) { mTests.push_back(test); }
  TestList &GetTests() { return mTests; }
  void Sort();
  void PrintScores(Score score, bool SkipWorseThanBaseline = false) const;

private:
  TestList mTests;
};

struct Case {
  Case(const char *fname, void (*dtoa)(double, char *), bool baseline = false,
       bool fake = false)
      : fname(fname), dtoa(dtoa), baseline(baseline), fake(fake) {
    TestManager::Instance().AddTest(this);
  }

  friend bool operator<(const Case &a, const Case &b) {
    return strcmp(a.fname, b.fname) < 0;
  }

  const char *fname;
  void (*dtoa)(double, char *);
  const bool baseline, fake;

  double min, max, sum, rms;
  unsigned count;

  void reset() {
    min = std::numeric_limits<double>::max();
    max = 0.0;
    rms = 0.0;
    sum = 0.0;
    count = 0;
  }
  void account(const double duration) {
    min = std::min(min, duration);
    max = std::max(max, duration);
    sum += duration;
    rms = std::sqrt((rms * rms * count + duration * duration) / (count += 1));
  }
};

#define STRINGIFY(x) #x
#define REGISTER_TEST(f) static Case gRegister##f(STRINGIFY(f), dtoa##_##f)
