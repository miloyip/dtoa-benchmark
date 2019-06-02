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

#include "erthink_arch.h"
#include "erthink_bswap.h"
#include "erthink_clz.h"
#include "erthink_defs.h"
#include "erthink_endian.h"
#include "erthink_intrin.h"

#include "testing.h"

//------------------------------------------------------------------------------

enum class foo { bar1, bar2, bar3 };
DEFINE_ENUM_FLAG_OPERATORS(foo)

//------------------------------------------------------------------------------

// LY: workaround for CLANG & GCC8
#define INT64_LITERAL(x) int64_t(INT64_C(x))

TEST(ops, fallback_clz32) {
  EXPECT_EQ(31, erthink::fallback_clz32(1));
  const uint32_t all = ~UINT32_C(0);
  for (int i = 0; i < 32; ++i) {
    SCOPED_TRACE("i = " + std::to_string(i));
    uint32_t bit = UINT32_C(1) << i;
    EXPECT_EQ(31 - i, erthink::fallback_clz32(bit));
    EXPECT_EQ((i == 31) ? 1 : 0, erthink::fallback_clz32(~bit));
    EXPECT_EQ(i, erthink::fallback_clz32(all >> i));
  }
}

__dll_export __noinline int _clz32(uint32_t u) { return erthink::clz32(u); }
TEST(ops, clz32) {
  EXPECT_EQ(31, _clz32(1));
  const uint32_t all = ~UINT32_C(0);
  for (int i = 0; i < 32; ++i) {
    SCOPED_TRACE("i = " + std::to_string(i));
    uint32_t bit = UINT32_C(1) << i;
    EXPECT_EQ(31 - i, _clz32(bit));
    EXPECT_EQ((i == 31) ? 1 : 0, _clz32(~bit));
    EXPECT_EQ(i, _clz32(all >> i));
  }
}

//------------------------------------------------------------------------------

TEST(ops, fallback_clz64) {
  EXPECT_EQ(63, erthink::fallback_clz64(1));
  const uint64_t all = ~UINT64_C(0);
  for (int i = 0; i < 64; ++i) {
    SCOPED_TRACE("i = " + std::to_string(i));
    uint64_t bit = UINT64_C(1) << i;
    EXPECT_EQ(63 - i, erthink::fallback_clz64(bit));
    EXPECT_EQ((i == 63) ? 1 : 0, erthink::fallback_clz64(~bit));
    EXPECT_EQ(i, erthink::fallback_clz64(all >> i));
  }
}

__dll_export __noinline int _clz64(uint64_t u) { return erthink::clz64(u); }
TEST(ops, clz64) {
  EXPECT_EQ(63, _clz64(1));
  const uint64_t all = ~UINT64_C(0);
  for (int i = 0; i < 64; ++i) {
    SCOPED_TRACE("i = " + std::to_string(i));
    uint64_t bit = UINT64_C(1) << i;
    EXPECT_EQ(63 - i, _clz64(bit));
    EXPECT_EQ((i == 63) ? 1 : 0, _clz64(~bit));
    EXPECT_EQ(i, _clz64(all >> i));
  }
}

//------------------------------------------------------------------------------

TEST(ops, bswap) {
  EXPECT_EQ(1, erthink::bswap<uint8_t>(1));
  EXPECT_EQ(2, erthink::bswap<int8_t>(2));

  EXPECT_EQ(UINT16_C(0x3412), erthink::bswap<uint16_t>(UINT16_C(0x1234)));
  EXPECT_EQ(INT16_C(0x7856), erthink::bswap<int16_t>(INT16_C(0x5678)));

  EXPECT_EQ(UINT32_C(0x78563412),
            erthink::bswap<uint32_t>(UINT32_C(0x12345678)));
  EXPECT_EQ(INT32_C(0x12345678), erthink::bswap<int32_t>(INT32_C(0x78563412)));

  EXPECT_EQ(UINT64_C(0xf0debc9a78563412),
            erthink::bswap<uint64_t>(UINT64_C(0x123456789abcdef0)));
  EXPECT_EQ(INT64_LITERAL(0x123456789abcdef0),
            erthink::bswap<int64_t>(INT64_C(0xf0debc9a78563412)));
}

TEST(ops, endian) {
  EXPECT_EQ(128, erthink::h2be<uint8_t>(erthink::h2le<uint8_t>(128)));
  EXPECT_EQ(-42, erthink::h2be<int8_t>(erthink::h2le<int8_t>(-42)));
  EXPECT_EQ(128, erthink::be2h<uint8_t>(erthink::le2h<uint8_t>(128)));
  EXPECT_EQ(-42, erthink::be2h<int8_t>(erthink::le2h<int8_t>(-42)));

  EXPECT_EQ(UINT16_C(0x3412),
            erthink::h2be<uint16_t>(erthink::h2le<uint16_t>(UINT16_C(0x1234))));
  EXPECT_EQ(INT16_C(0x7856),
            erthink::h2be<int16_t>(erthink::h2le<int16_t>(INT16_C(0x5678))));
  EXPECT_EQ(UINT16_C(0x3412),
            erthink::be2h<uint16_t>(erthink::le2h<uint16_t>(UINT16_C(0x1234))));
  EXPECT_EQ(INT16_C(0x7856),
            erthink::be2h<int16_t>(erthink::le2h<int16_t>(INT16_C(0x5678))));

  EXPECT_EQ(UINT16_C(0x1234),
            erthink::le2h<uint16_t>(erthink::h2le<uint16_t>(UINT16_C(0x1234))));
  EXPECT_EQ(INT16_C(0x5678),
            erthink::le2h<int16_t>(erthink::h2le<int16_t>(INT16_C(0x5678))));
  EXPECT_EQ(UINT16_C(0x1234),
            erthink::le2h<uint16_t>(erthink::h2le<uint16_t>(UINT16_C(0x1234))));
  EXPECT_EQ(INT16_C(0x5678),
            erthink::le2h<int16_t>(erthink::h2le<int16_t>(INT16_C(0x5678))));

  EXPECT_EQ(UINT16_C(0x1234),
            erthink::be2h<uint16_t>(erthink::h2be<uint16_t>(UINT16_C(0x1234))));
  EXPECT_EQ(INT16_C(0x5678),
            erthink::be2h<int16_t>(erthink::h2be<int16_t>(INT16_C(0x5678))));
  EXPECT_EQ(UINT16_C(0x1234),
            erthink::be2h<uint16_t>(erthink::h2be<uint16_t>(UINT16_C(0x1234))));
  EXPECT_EQ(INT16_C(0x5678),
            erthink::be2h<int16_t>(erthink::h2be<int16_t>(INT16_C(0x5678))));

  EXPECT_EQ(
      UINT32_C(0x78563412),
      erthink::h2be<uint32_t>(erthink::h2le<uint32_t>(UINT32_C(0x12345678))));
  EXPECT_EQ(INT32_C(0x12345678), erthink::h2be<int32_t>(erthink::h2le<int32_t>(
                                     INT32_C(0x78563412))));
  EXPECT_EQ(
      UINT32_C(0x78563412),
      erthink::be2h<uint32_t>(erthink::le2h<uint32_t>(UINT32_C(0x12345678))));
  EXPECT_EQ(INT32_C(0x12345678), erthink::be2h<int32_t>(erthink::le2h<int32_t>(
                                     INT32_C(0x78563412))));

  EXPECT_EQ(
      UINT32_C(0x12345678),
      erthink::le2h<uint32_t>(erthink::h2le<uint32_t>(UINT32_C(0x12345678))));
  EXPECT_EQ(INT32_C(0x78563412), erthink::le2h<int32_t>(erthink::h2le<int32_t>(
                                     INT32_C(0x78563412))));
  EXPECT_EQ(
      UINT32_C(0x12345678),
      erthink::le2h<uint32_t>(erthink::h2le<uint32_t>(UINT32_C(0x12345678))));
  EXPECT_EQ(INT32_C(0x78563412), erthink::le2h<int32_t>(erthink::h2le<int32_t>(
                                     INT32_C(0x78563412))));

  EXPECT_EQ(
      UINT32_C(0x12345678),
      erthink::be2h<uint32_t>(erthink::h2be<uint32_t>(UINT32_C(0x12345678))));
  EXPECT_EQ(INT32_C(0x78563412), erthink::be2h<int32_t>(erthink::h2be<int32_t>(
                                     INT32_C(0x78563412))));
  EXPECT_EQ(
      UINT32_C(0x12345678),
      erthink::be2h<uint32_t>(erthink::h2be<uint32_t>(UINT32_C(0x12345678))));
  EXPECT_EQ(INT32_C(0x78563412), erthink::be2h<int32_t>(erthink::h2be<int32_t>(
                                     INT32_C(0x78563412))));

  EXPECT_EQ(UINT64_C(0xf0debc9a78563412),
            erthink::h2be<uint64_t>(
                erthink::h2le<uint64_t>(UINT64_C(0x123456789abcdef0))));
  EXPECT_EQ(INT64_LITERAL(0x123456789abcdef0),
            erthink::h2be<int64_t>(
                erthink::h2le<int64_t>(INT64_C(0xf0debc9a78563412))));
  EXPECT_EQ(UINT64_C(0xf0debc9a78563412),
            erthink::be2h<uint64_t>(
                erthink::le2h<uint64_t>(UINT64_C(0x123456789abcdef0))));
  EXPECT_EQ(INT64_LITERAL(0x123456789abcdef0),
            erthink::be2h<int64_t>(
                erthink::le2h<int64_t>(INT64_C(0xf0debc9a78563412))));

  EXPECT_EQ(UINT64_C(0x123456789abcdef0),
            erthink::le2h<uint64_t>(
                erthink::h2le<uint64_t>(UINT64_C(0x123456789abcdef0))));

  EXPECT_EQ(INT64_LITERAL(0xf0debc9a78563412),
            erthink::le2h<int64_t>(
                erthink::h2le<int64_t>(INT64_C(0xf0debc9a78563412))));
  EXPECT_EQ(UINT64_C(0x123456789abcdef0),
            erthink::le2h<uint64_t>(
                erthink::h2le<uint64_t>(UINT64_C(0x123456789abcdef0))));
  EXPECT_EQ(INT64_LITERAL(0xf0debc9a78563412),
            erthink::le2h<int64_t>(
                erthink::h2le<int64_t>(INT64_C(0xf0debc9a78563412))));

  EXPECT_EQ(UINT64_C(0x123456789abcdef0),
            erthink::be2h<uint64_t>(
                erthink::h2be<uint64_t>(UINT64_C(0x123456789abcdef0))));
  EXPECT_EQ(INT64_LITERAL(0xf0debc9a78563412),
            erthink::be2h<int64_t>(
                erthink::h2be<int64_t>(INT64_C(0xf0debc9a78563412))));
  EXPECT_EQ(UINT64_C(0x123456789abcdef0),
            erthink::be2h<uint64_t>(
                erthink::h2be<uint64_t>(UINT64_C(0x123456789abcdef0))));
  EXPECT_EQ(INT64_LITERAL(0xf0debc9a78563412),
            erthink::be2h<int64_t>(
                erthink::h2be<int64_t>(INT64_C(0xf0debc9a78563412))));
}

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
