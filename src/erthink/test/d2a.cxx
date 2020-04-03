/*
 *  Copyright (c) 1994-2020 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/erthink/erthink
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "testing.h"

#include "erthink_d2a.h"
#include "erthink_defs.h"

#ifdef _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable : 4710) /* function not inlined */
#endif
#include <cfloat> // for FLT_MAX, etc
#include <cmath>  // for M_PI, etc
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------------

struct P {
  const long double v;
  P(const long double v) : v(v) {}
  friend bool operator==(const P &a, const P &b) { return a.v == b.v; }
  friend bool operator!=(const P &a, const P &b) { return a.v != b.v; }
  friend std::ostream &operator<<(std::ostream &os, const P &v) {
    return os << std::setprecision(19) << v.v << '(' << std::hexfloat << v.v
              << ')';
  }
};

template <typename T> struct d2a : public ::testing::Test {
  static constexpr bool accurate = T::value;
  static __hot __noinline char *convert(const double value, char *ptr) {
    return erthink::d2a<accurate>(value, ptr);
  }

  static std::string mantissa_str_map(const char *str,
                                      const bool strip_tail_0) {
    const size_t len = strlen(str);
    assert(len < 128);

    std::string r;
    r.reserve(len);

    size_t i;
    for (i = 0; i < len; ++i) {
      if (str[i] >= '0' && str[i] <= '9') {
        if (str[i] != '0' || !r.empty())
          r.push_back(i);
      } else if (str[i] != '-' && str[i] != '.')
        break;
    }

    if (r.empty() && i > 0 && str[i - 1] == '0')
      r.push_back(i - 1);
    if (strip_tail_0)
      while (i > 0 && str[i - 1] == '0' && r.size() > 1) {
        r.pop_back();
        --i;
      }
    return r;
  }

  static std::tuple<bool, int, int> mantissa_str_diff(const char *a,
                                                      const std::string &ma,
                                                      const char *b,
                                                      const std::string &mb) {
    size_t i, j;
    for (i = j = 0;;) {
      const bool a_end = i >= ma.size();
      const bool b_end = j >= mb.size();
      if (a_end && b_end)
        break;

      const char a_digit = a_end ? '0' : a[int(ma[i])];
      const char b_digit = b_end ? '0' : b[int(mb[j])];
      if (a_digit != b_digit)
        return std::make_tuple(true, a_end ? ma.back() : ma[i],
                               b_end ? mb.back() : mb[j]);

      i += !a_end;
      j += !b_end;
    }
    return std::make_tuple(false, 0, 0);
  }

  static bool make_shorter(char (&str)[erthink::d2a_max_chars + 1]) {
    if (str[0] == '-')
      return false;

    int i = 0;
    while (str[i] >= '0' && str[i] <= '9')
      ++i;
    bool have_dot = false;
    if (str[i] == '.') {
      do
        ++i;
      while (str[i] >= '0' && str[i] <= '9');
      have_dot = true;
    }
    if (i < 2 || str[i] != 'e')
      return false;

    int j = i - 1;
    j -= str[j] == '.';
    if (j < 1)
      return false;

    if (str[j] >= '5') {
      j -= 1;
      j -= str[j] == '.';
      for (;;) {
        assert(j >= 0 && str[j] != '.');
        if (str[j] < '9') {
          str[j] += 1;
          break;
        }
        str[j] = '0';
        j -= 1;
        j -= (j >= 0 && str[j] == '.');
        if (j < 0) {
          j = str[0] == '.';
          str[j] = '1';
          have_dot = false;
          break;
        }
      }
    }

    if (have_dot)
      memmove(str + i - 1, str + i, erthink::d2a_max_chars - i);
    else
      snprintf(str + i - 1, erthink::d2a_max_chars - i + 1, "e%i",
               atoi(str + i + 1) + 1);
    return true;
  }

  void probe_d2a(char (&buffer)[erthink::d2a_max_chars + 1],
                 const double value) {
    char *d2a_end = convert(value, buffer);
    ASSERT_LT(buffer, d2a_end);
    ASSERT_GT(erthink::array_end(buffer), d2a_end);
    *d2a_end = '\0';

    char *strtod_end = nullptr;
    double probe = strtod(buffer, &strtod_end);
    EXPECT_EQ(d2a_end, strtod_end);

#ifdef TEST_D2A_CHECK_LAST_DIGIT_ROUNDING
    const std::string ma = mantissa_str_map(buffer, true);

    char print_buffer[32];
    snprintf(print_buffer, sizeof(print_buffer), "%.*e", int(ma.size()) - 1,
             value);
    const std::string mb = mantissa_str_map(print_buffer, false);
    const auto diff = mantissa_str_diff(buffer, ma, print_buffer, mb);
    if (std::get<0>(diff)) {
      printf("d2a:%s <> printf:%s\n"
             "%*c%*c\n"
             "%*c%*.*e\n",
             buffer, print_buffer, std::get<1>(diff) + 5, '^',
             std::get<2>(diff) + 11 - std::get<1>(diff) + (int)strlen(buffer),
             '|', std::get<1>(diff) + 5,
             (buffer[std::get<1>(diff)] < print_buffer[std::get<2>(diff)])
                 ? '<'
                 : '>',
             int(strlen(buffer)) + 12 + int(strlen(print_buffer)) -
                 std::get<1>(diff),
             int(mb.size() + 1), value);
      fflush(nullptr);
      fflush(nullptr);
    }
#endif /* TEST_D2A_CHECK_LAST_DIGIT_ROUNDING */
    EXPECT_EQ(P(value), P(probe));
    ensure_shortest(value, buffer);
  }

  bool probe_d2a(uint64_t u64, char (&buffer)[erthink::d2a_max_chars + 1]) {
    const double f64 = erthink::grisu::cast(u64);
    switch (std::fpclassify(f64)) {
    case FP_NAN:
    case FP_INFINITE:
      return false;
    default:
      probe_d2a(buffer, f64);
    }

    const float f32 = static_cast<float>(f64);
    switch (std::fpclassify(f32)) {
    case FP_NAN:
    case FP_INFINITE:
      return false;
    default:
      probe_d2a(buffer, f32);
      return true;
    }
  }

  void ensure_shortest(const double value,
                       char (&buf)[erthink::d2a_max_chars + 1]) {
#ifndef TEST_D2A_SHORTEST_VALIDATION
    (void)value;
    (void)buf;
#else
    char str[erthink::d2a_max_chars + 1];
    memcpy(str, buf, sizeof(str));

    if (make_shorter(str)) {
      char *strtod_end = nullptr;
      double probe = strtod(str, &strtod_end);
      ASSERT_EQ(*strtod_end, '\0');
      EXPECT_NE(P(value), P(probe));
    }
#endif /* TEST_D2A_SHORTEST_VALIDATION */
  }
};

#ifdef TYPED_TEST_SUITE_P
TYPED_TEST_SUITE_P(d2a);
#else
TYPED_TEST_CASE_P(d2a);
#endif

//------------------------------------------------------------------------------

TYPED_TEST_P(d2a, trivia) {
  char buffer[erthink::d2a_max_chars + 1];
  char *end = TestFixture::convert(0, buffer);
  EXPECT_EQ(1, end - buffer);
  EXPECT_EQ(buffer[0], '0');

  TestFixture::probe_d2a(buffer, 0.0);
  TestFixture::probe_d2a(buffer, 1.0);
  TestFixture::probe_d2a(buffer, 2.0);
  TestFixture::probe_d2a(buffer, 3.0);
  TestFixture::probe_d2a(buffer, -0.0);
  TestFixture::probe_d2a(buffer, -1.0);
  TestFixture::probe_d2a(buffer, -2.0);
  TestFixture::probe_d2a(buffer, -3.0);
  TestFixture::probe_d2a(buffer, M_PI);
  TestFixture::probe_d2a(buffer, -M_PI);

  TestFixture::probe_d2a(buffer, INT32_MIN);
  TestFixture::probe_d2a(buffer, INT32_MAX);
  TestFixture::probe_d2a(buffer, UINT16_MAX);
  TestFixture::probe_d2a(buffer, UINT32_MAX);
  TestFixture::probe_d2a(buffer, FLT_MAX);
  TestFixture::probe_d2a(buffer, -FLT_MAX);
  TestFixture::probe_d2a(buffer, FLT_MAX);
  TestFixture::probe_d2a(buffer, -FLT_MAX);
  TestFixture::probe_d2a(buffer, FLT_MIN);
  TestFixture::probe_d2a(buffer, -FLT_MIN);
  TestFixture::probe_d2a(buffer, FLT_MAX * M_PI);
  TestFixture::probe_d2a(buffer, -FLT_MAX * M_PI);
  TestFixture::probe_d2a(buffer, FLT_MIN * M_PI);
  TestFixture::probe_d2a(buffer, -FLT_MIN * M_PI);

  TestFixture::probe_d2a(buffer, DBL_MAX);
  TestFixture::probe_d2a(buffer, -DBL_MAX);
  TestFixture::probe_d2a(buffer, DBL_MIN);
  TestFixture::probe_d2a(buffer, -DBL_MIN);
  TestFixture::probe_d2a(buffer, DBL_MAX / M_PI);
  TestFixture::probe_d2a(buffer, -DBL_MAX / M_PI);
  TestFixture::probe_d2a(buffer, DBL_MIN * M_PI);
  TestFixture::probe_d2a(buffer, -DBL_MIN * M_PI);
}

TYPED_TEST_P(d2a, stairwell) {
  char buffer[erthink::d2a_max_chars + 1];
  TestFixture::probe_d2a(UINT64_C(4989988387303176888), buffer);
  TestFixture::probe_d2a(UINT64_C(4895412794877399892), buffer);
  TestFixture::probe_d2a(UINT64_C(13717964465041107931), buffer);
  TestFixture::probe_d2a(UINT64_C(13416223289762161370), buffer);
  TestFixture::probe_d2a(UINT64_C(13434237688651515774), buffer);
  TestFixture::probe_d2a(UINT64_C(5008002785836588600), buffer);
  TestFixture::probe_d2a(UINT64_C(4210865651786747166), buffer);
  TestFixture::probe_d2a(UINT64_C(14231374822717078073), buffer);
  TestFixture::probe_d2a(UINT64_C(13434237688189056622), buffer);
  TestFixture::probe_d2a(UINT64_C(13717964465155820979), buffer);
  TestFixture::probe_d2a(UINT64_C(4237887249171175423), buffer);
  TestFixture::probe_d2a(UINT64_C(13632396072180810313), buffer);

  const double up = 1.1283791670955125739 /* 2/sqrt(pi) */;
  for (double value = DBL_MIN * up; value < DBL_MAX / up; value *= up) {
    TestFixture::probe_d2a(buffer, value);
    const float f32 = static_cast<float>(value);
    if (!std::isinf(f32))
      TestFixture::probe_d2a(buffer, f32);
  }

  const double down = 0.91893853320467274178 /* ln(sqrt(2pi)) */;
  for (double value = DBL_MAX * down; value > DBL_MIN / down; value *= down) {
    TestFixture::probe_d2a(buffer, value);
    const float f32 = static_cast<float>(value);
    if (!std::isinf(f32))
      TestFixture::probe_d2a(buffer, f32);
  }

  for (uint64_t mantissa = erthink::grisu::IEEE754_DOUBLE_MANTISSA_MASK;
       mantissa != 0; mantissa >>= 1) {
    for (uint64_t offset = 1; offset < mantissa; offset <<= 1) {
      for (uint64_t exp = 0;
           exp <= erthink::grisu::IEEE754_DOUBLE_EXPONENT_MASK;
           exp += erthink::grisu::IEEE754_DOUBLE_IMPLICIT_LEAD) {
        for (uint64_t bit = mantissa; bit != 0;) {
          bit >>= 1;
          TestFixture::probe_d2a((mantissa + offset) ^ exp ^ bit, buffer);
          TestFixture::probe_d2a((mantissa - offset) ^ exp ^ bit, buffer);
        }
        if (GTEST_IS_EXECUTION_TIMEOUT())
          break;
      }
    }
  }
}

TYPED_TEST_P(d2a, random3e7) {
  char buffer[erthink::d2a_max_chars + 1];
  uint64_t prng(uint64_t(time(0)));
  SCOPED_TRACE("PGNG seed=" + std::to_string(prng));
  for (int i = 0; i < 33333333;) {
    i += TestFixture::probe_d2a(prng, buffer);
    prng *= UINT64_C(6364136223846793005);
    prng += UINT64_C(1442695040888963407);
    if (GTEST_IS_EXECUTION_TIMEOUT())
      break;
  }
}

//------------------------------------------------------------------------------

#ifdef REGISTER_TYPED_TEST_SUITE_P
REGISTER_TYPED_TEST_SUITE_P(d2a, trivia, stairwell, random3e7);
INSTANTIATE_TYPED_TEST_SUITE_P(accurate, d2a, std::true_type);
INSTANTIATE_TYPED_TEST_SUITE_P(fast, d2a, std::false_type);
#endif

runtime_limiter ci_runtime_limiter;

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
