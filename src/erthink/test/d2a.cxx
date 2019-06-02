/*
 *  Copyright (c) 1994-2019 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/leo-yuriev/erthink
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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include "erthink_d2a.h"
#include "erthink_defs.h"
#include "testing.h"

#ifdef _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable : 4710) /* function not inlined */
#endif
#include <cfloat> // for FLT_MAX, etc
#include <cmath>  // for M_PI, etc
#ifdef _MSC_VER
#pragma warning(pop)
#endif

__hot __dll_export __noinline char *_d2a(const double value, char *ptr) {
  return erthink::d2a(value, ptr);
}

//------------------------------------------------------------------------------

static void probe_d2a(char (&buffer)[23 + 1], const double value) {
  char *d2a_end = _d2a(value, buffer);
  ASSERT_LT(buffer, d2a_end);
  ASSERT_GT(erthink::array_end(buffer), d2a_end);
  *d2a_end = '\0';

  char *strtod_end = nullptr;
  double probe = strtod(buffer, &strtod_end);
  EXPECT_EQ(d2a_end, strtod_end);
  EXPECT_EQ(value, probe);
}

TEST(d2a, trivia) {
  char buffer[23 + 1];
  char *end = _d2a(0, buffer);
  EXPECT_EQ(1, end - buffer);
  EXPECT_EQ(buffer[0], '0');

  probe_d2a(buffer, 0.0);
  probe_d2a(buffer, 1.0);
  probe_d2a(buffer, 2.0);
  probe_d2a(buffer, 3.0);
  probe_d2a(buffer, -0.0);
  probe_d2a(buffer, -1.0);
  probe_d2a(buffer, -2.0);
  probe_d2a(buffer, -3.0);
  probe_d2a(buffer, M_PI);
  probe_d2a(buffer, -M_PI);

  probe_d2a(buffer, INT32_MIN);
  probe_d2a(buffer, INT32_MAX);
  probe_d2a(buffer, UINT16_MAX);
  probe_d2a(buffer, UINT32_MAX);
  probe_d2a(buffer, FLT_MAX);
  probe_d2a(buffer, -FLT_MAX);
  probe_d2a(buffer, FLT_MAX);
  probe_d2a(buffer, -FLT_MAX);
  probe_d2a(buffer, FLT_MIN);
  probe_d2a(buffer, -FLT_MIN);
  probe_d2a(buffer, FLT_MAX * M_PI);
  probe_d2a(buffer, -FLT_MAX * M_PI);
  probe_d2a(buffer, FLT_MIN * M_PI);
  probe_d2a(buffer, -FLT_MIN * M_PI);

  probe_d2a(buffer, DBL_MAX);
  probe_d2a(buffer, -DBL_MAX);
  probe_d2a(buffer, DBL_MIN);
  probe_d2a(buffer, -DBL_MIN);
  probe_d2a(buffer, DBL_MAX / M_PI);
  probe_d2a(buffer, -DBL_MAX / M_PI);
  probe_d2a(buffer, DBL_MIN * M_PI);
  probe_d2a(buffer, -DBL_MIN * M_PI);
}

static bool probe_d2a(uint64_t u64, char (&buffer)[23 + 1]) {
  erthink::grisu::casting_union casting(u64);
  switch (std::fpclassify(casting.f)) {
  case FP_NAN:
  case FP_INFINITE:
    return false;
  default:
    probe_d2a(buffer, casting.f);
  }

  const float f32 = static_cast<float>(casting.f);
  switch (std::fpclassify(f32)) {
  case FP_NAN:
  case FP_INFINITE:
    return false;
  default:
    probe_d2a(buffer, f32);
    return true;
  }
}

TEST(d2a, stairwell) {
  char buffer[23 + 1];
  probe_d2a(UINT64_C(4989988387303176888), buffer);
  probe_d2a(UINT64_C(4895412794877399892), buffer);
  probe_d2a(UINT64_C(13717964465041107931), buffer);
  probe_d2a(UINT64_C(13416223289762161370), buffer);
  probe_d2a(UINT64_C(13434237688651515774), buffer);
  probe_d2a(UINT64_C(5008002785836588600), buffer);
  probe_d2a(UINT64_C(4210865651786747166), buffer);
  probe_d2a(UINT64_C(14231374822717078073), buffer);
  probe_d2a(UINT64_C(13434237688189056622), buffer);
  probe_d2a(UINT64_C(13717964465155820979), buffer);
  probe_d2a(UINT64_C(4237887249171175423), buffer);
  probe_d2a(UINT64_C(13632396072180810313), buffer);

  const double up = 1.1283791670955125739 /* 2/sqrt(pi) */;
  for (double value = DBL_MIN * up; value < DBL_MAX / up; value *= up) {
    probe_d2a(buffer, value);
    const float f32 = static_cast<float>(value);
    if (!std::isinf(f32))
      probe_d2a(buffer, f32);
  }

  const double down = 0.91893853320467274178 /* ln(sqrt(2pi)) */;
  for (double value = DBL_MAX * down; value > DBL_MIN / down; value *= down) {
    probe_d2a(buffer, value);
    const float f32 = static_cast<float>(value);
    if (!std::isinf(f32))
      probe_d2a(buffer, f32);
  }

  for (uint64_t mantissa = erthink::grisu::IEEE754_DOUBLE_MANTISSA_MASK;
       mantissa != 0; mantissa >>= 1) {
    for (uint64_t offset = 1; offset < mantissa; offset <<= 1) {
      for (uint64_t exp = 0;
           exp <= erthink::grisu::IEEE754_DOUBLE_EXPONENT_MASK;
           exp += erthink::grisu::IEEE754_DOUBLE_IMPLICIT_LEAD) {
        probe_d2a((mantissa + offset) ^ exp, buffer);
        probe_d2a((mantissa - offset) ^ exp, buffer);
      }
    }
  }
}

TEST(d2a, random3e6) {
  char buffer[23 + 1];
  uint64_t prng(uint64_t(time(0)));
  SCOPED_TRACE("PGNG seed=" + std::to_string(prng));
  for (int i = 0; i < 3333333;) {
    i += probe_d2a(prng, buffer);
    prng *= UINT64_C(6364136223846793005);
    prng += UINT64_C(1442695040888963407);
  }
}

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
