/*
 *  Copyright (c) 1994-2019 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/leo-yuriev/erthink
 *  ZLib License
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgement in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 */

#include "erthink_arch.h"
#include "erthink_clz.h"
#include "erthink_defs.h"
#include "erthink_intrin.h"

#include "testing.h"

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
