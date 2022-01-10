/*
 *  Copyright (c) 1994-2021 Leonid Yuriev <leo@yuriev.ru>.
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

#define ERTHINK_USE_NATIVE_128 0

#include "testing.h++"

#include "erthink.h++"
#include <algorithm> // for std::random_shuffle
#include <cfloat>    // for DBL_MAX
#include <numeric>   // for std::iota
#include <sstream>

#include "autogen_u128_basic.h++"

//------------------------------------------------------------------------------

TEST(u128, to_string) {
  ASSERT_EQ(std::to_string(std::numeric_limits<erthink::uint128_t>::max()),
            "340282366920938463463374607431768211455");
  ASSERT_EQ(std::to_string(std::numeric_limits<erthink::uint128_t>::min()),
            "0");
  ASSERT_EQ(std::to_string(erthink::uint128_t(142)), "142");
  ASSERT_EQ(std::to_string(erthink::uint128_t(42), 16), "2a");
  ASSERT_EQ(std::to_string(erthink::uint128_t(57), 8), "71");
  ASSERT_EQ(std::to_string(erthink::uint128_t(UINT64_C(0x6E10784D412040D),
                                              UINT64_C(0xFF39F12CF4081907))),
            "9143787268497110792970552074639513863");
  ASSERT_EQ(std::to_string(erthink::uint128_t(~UINT64_C(0))),
            "18446744073709551615");
  ASSERT_EQ(std::to_string(erthink::uint128_t(~UINT64_C(0), 0)),
            "340282366920938463444927863358058659840");
  ASSERT_EQ(std::to_string(erthink::uint128_t(UINT64_C(0x90770897eb39d46c), 0)),
            "192026889014721788266898567285392277504");
  ASSERT_EQ(std::to_string(erthink::uint128_t(UINT64_C(0xb0ec5848ef24a556))),
            "12748661715452077398");
}

TEST(u128, from_string) {
  using erthink::operator"" _u128;
  ASSERT_EQ(340282366920938463463374607431768211455_u128,
            std::numeric_limits<erthink::uint128_t>::max());
  ASSERT_EQ(0_u128, std::numeric_limits<erthink::uint128_t>::min());
  ASSERT_EQ(142_u128, erthink::uint128_t(142));
  ASSERT_EQ(0x2A_u128, erthink::uint128_t(42));
  ASSERT_EQ(071_u128, erthink::uint128_t(57));
  ASSERT_EQ(9143787268497110792970552074639513863_u128,
            erthink::uint128_t(UINT64_C(0x6E10784D412040D),
                               UINT64_C(0xFF39F12CF4081907)));
  ASSERT_EQ(18446744073709551615_u128, erthink::uint128_t(~UINT64_C(0)));
  ASSERT_EQ(340282366920938463444927863358058659840_u128,
            erthink::uint128_t(~UINT64_C(0), 0));
  ASSERT_EQ(192026889014721788266898567285392277504_u128,
            erthink::uint128_t(UINT64_C(0x90770897eb39d46c), 0));
  ASSERT_EQ(12748661715452077398_u128,
            erthink::uint128_t(UINT64_C(0xb0ec5848ef24a556)));
}

TEST(u128, from_chars) {
#if !ERTHINK_HAVE_std_to_chars
  GTEST_SKIP() << "SKIPPEND because of no std::to_chars<>";
#else
  const char *zero = "000000";
  const auto from_zero =
      erthink::uint128_t::from_chars(zero, zero + strlen(zero));
  ASSERT_EQ(std::get<0>(from_zero)[0], '\0');
  ASSERT_EQ(std::get<1>(from_zero), erthink::uint128_t(0));
  ASSERT_EQ(std::get<2>(from_zero), std::errc());

  const char *hex = "0x12345";
  const auto from_hex = erthink::uint128_t::from_chars(hex, hex + strlen(hex));
  ASSERT_EQ(std::get<0>(from_hex)[0], '\0');
  ASSERT_EQ(std::get<1>(from_hex), erthink::uint128_t(0x12345));
  ASSERT_EQ(std::get<2>(from_hex), std::errc());

  const char *max = "340282366920938463463374607431768211455";
  const auto from_max = erthink::uint128_t::from_chars(max, max + strlen(max));
  ASSERT_EQ(std::get<0>(from_max)[0], '\0');
  ASSERT_EQ(std::get<1>(from_max),
            std::numeric_limits<erthink::uint128_t>::max());
  ASSERT_EQ(std::get<2>(from_max), std::errc());

  const char *overflow = "0x340282366920938463463374607431768211455";
  const auto from_overflow =
      erthink::uint128_t::from_chars(overflow, overflow + strlen(overflow));
  ASSERT_EQ(std::get<0>(from_overflow)[0], '\0');
  ASSERT_EQ(std::get<2>(from_overflow), std::errc::result_out_of_range);

  const char *partial = "0123456789"; // octal literal with invalid '89'
  const auto from_partial =
      erthink::uint128_t::from_chars(partial, partial + strlen(partial));
  ASSERT_EQ(std::get<0>(from_partial), partial + 8);
  ASSERT_EQ(std::get<0>(from_partial)[0], '8');
  ASSERT_EQ(std::get<1>(from_partial), erthink::uint128_t(01234567));
  ASSERT_EQ(std::get<2>(from_partial), std::errc());

  const char *invalid = "invalid";
  const auto from_invalid =
      erthink::uint128_t::from_chars(invalid, invalid + strlen(invalid));
  ASSERT_EQ(std::get<0>(from_invalid), invalid);
  ASSERT_EQ(std::get<2>(from_invalid), std::errc::invalid_argument);
#endif /* ERTHINK_HAVE_std_to_chars */
}

TEST(u128, stream) {
  std::stringstream ss;
  const auto default_flags = ss.flags();
  ss << std::numeric_limits<erthink::uint128_t>::max();
  ASSERT_EQ(ss.str(), "340282366920938463463374607431768211455");

  ss.str("");
  ss << std::numeric_limits<erthink::uint128_t>::min();
  ASSERT_EQ(ss.str(), "0");

  ss.str("");
  ss.flags(default_flags);
  ss << std::setw(9) << 42;
  ASSERT_EQ(ss.str(), "       42");

  ss.str("");
  ss.flags(default_flags);
  ss << std::left << std::setw(5) << erthink::uint128_t(42);
  ASSERT_EQ(ss.str(), "42   ");

  ss.str("");
  ss.flags(default_flags);
  ss << std::setfill('_') << std::showbase << std::uppercase << std::oct
     << std::internal << std::setw(5) << erthink::uint128_t(42);
  ASSERT_EQ(ss.str(), "0__52");

  ss.str("");
  ss.flags(default_flags);
  ss << std::setfill('_') << std::showpos << std::showbase << std::oct
     << std::left << std::setw(5) << erthink::uint128_t(42);
  ASSERT_EQ(ss.str(), "+052_");

  ss.str("");
  ss.flags(default_flags);
  ss << std::setfill('_') << std::showbase << std::uppercase << std::hex
     << std::internal << std::setw(5) << erthink::uint128_t(42);
  ASSERT_EQ(ss.str(), "0X_2A");
}

//------------------------------------------------------------------------------

#ifdef ERTHINK_NATIVE_U128_TYPE
using native_u128 = ERTHINK_NATIVE_U128_TYPE;

struct uint64_lcg {
  using result_type = uint64_t;
  result_type state;

  constexpr uint64_lcg(uint64_t seed) noexcept : state(seed) {}
  static constexpr result_type min() noexcept { return 0; }
  static constexpr result_type max() noexcept { return ~result_type(0); }
  cxx14_constexpr result_type operator()() {
    const result_type r = (state += UINT64_C(1442695040888963407));
    state *= UINT64_C(6364136223846793005);
    return r;
  }
  cxx14_constexpr result_type operator()(size_t range) {
    return operator()() % range;
  }
};

static uint64_lcg lcg(uint64_t(time(0)));

static std::array<unsigned, 128> random_shuffle_0_127() noexcept {
  std::array<unsigned, 128> r;
  std::iota(r.begin(), r.end(), 0);
  std::shuffle(r.begin(), r.end(), lcg);
  return r;
}

static uint64_t N;

#ifdef DO_GENERATE_BASIC

#include <fstream>

static const char *value(const erthink::uint128_t &v) {
  static char buf[128];
  if (v.h == 0)
    snprintf(buf, sizeof(buf), "U128(UINT64_C(0x%016" PRIx64 "))", v.l);
  else if (v.h == UINT64_MAX && v.l > INT64_MAX)
    snprintf(buf, sizeof(buf), "U128(-INT64_C(%" PRIi64 "))", -int64_t(v));
  else
    snprintf(buf, sizeof(buf),
             "U128(UINT64_C(0x%016" PRIx64 "), UINT64_C(0x%016" PRIx64 "))",
             v.h, v.l);
  return buf;
}

static void replace(std::string &oper, const std::string &var,
                    const std::string &val) {
  const auto pos = oper.find(var);
  if (pos != std::string::npos)
    oper.replace(pos, var.length(), val);
}

static void gen_expect(std::ostream &out, const char *_oper, const unsigned s,
                       const bool x, const bool y) {
  assert(x == y);
  std::string oper(_oper);

  replace(oper, "!(!", "(");
  out << "  EXPECT_" << (x ? "TRUE" : "FALSE") << oper << ";\n";
  (void)s;
  (void)y;
}

static void gen_expect(std::ostream &out, const char *_oper, const unsigned s,
                       const erthink::uint128_t x, const native_u128 y) {
  assert(native_u128(x) == y);
  std::string oper(_oper);

  replace(oper, "S", std::to_string(s));
  if (strchr(_oper, 'T') && oper != "(T)")
    replace(oper, "(", "(T = A, ");
  else if (oper.front() == '(' && oper.back() == ')')
    oper = oper.substr(1, oper.size() - 2);

  out << "  EXPECT_EQ(" << oper << ", " << value(x) << ");\n";
  (void)y;
}

#define OUT(X, Y) gen_expect(out, STRINGIFY(X), S, X, Y)

static void gen(std::ostream &out, const erthink::uint128_t &A,
                const erthink::uint128_t &B) {
  static unsigned n;
  using U128 = erthink::uint128_t;
  out << "\nTEST(u128, autogen_basic_" << n++ << ") {\n";
  out << "  using U128 = erthink::uint128_t;\n";
  out << "  const U128 A = " << value(A) << ";\n";
  out << "  const U128 B = " << value(B) << ";\n";
  out << "  U128 T;\n";
  const auto S = unsigned(B) & 127;
  auto T = A;

  OUT((A > B), (native_u128(A) > native_u128(B)));
  OUT((A >= B), (native_u128(A) >= native_u128(B)));
  OUT((A == B), (native_u128(A) == native_u128(B)));
  OUT((A != B), (native_u128(A) != native_u128(B)));
  OUT((A < B), (native_u128(A) < native_u128(B)));
  OUT((A <= B), (native_u128(A) <= native_u128(B)));

  OUT((A + B), native_u128(A) + native_u128(B));
  OUT((A - B), native_u128(A) - native_u128(B));
  OUT((A ^ B), native_u128(A) ^ native_u128(B));
  OUT((A | B), native_u128(A) | native_u128(B));
  OUT((A & B), native_u128(A) & native_u128(B));
  OUT((A * B), native_u128(A) * native_u128(B));

  OUT((-A), -native_u128(A));
  OUT((~A), ~native_u128(A));
  OUT(!(!A), !!native_u128(A));

  if (B) {
    OUT(U128::divmod(A, B).first, native_u128(A) / native_u128(B));
    OUT(U128::divmod(A, B).second, native_u128(A) % native_u128(B));
  }

  OUT((A >> S), native_u128(A) >> S);
  OUT((A << S), native_u128(A) << S);

  T = A;
  OUT((T += B), native_u128(A) + native_u128(B));
  T = A;
  OUT((T -= B), native_u128(A) - native_u128(B));
  T = A;
  OUT((T ^= B), native_u128(A) ^ native_u128(B));
  T = A;
  OUT((T |= B), native_u128(A) | native_u128(B));
  T = A;
  OUT((T &= B), native_u128(A) & native_u128(B));
  T = A;
  OUT((T *= B), native_u128(A) * native_u128(B));

  if (B) {
    T = A;
    OUT((T /= B), native_u128(A) / native_u128(B));
    T = A;
    OUT((T %= B), native_u128(A) % native_u128(B));
  }

  T = A;
  OUT((T >>= S), native_u128(A) >> S);
  T = A;
  OUT((T <<= S), native_u128(A) << S);

  OUT((ror(A, S)), erthink::ror(native_u128(A), S));
  OUT((rol(A, S)), erthink::rol(native_u128(A), S));

  T = A;
  OUT((T++), native_u128(A));
  OUT((T), native_u128(A) + 1);
  T = A;
  OUT((T--), native_u128(A));
  OUT((T), native_u128(A) - 1);
  T = A;
  OUT((++T), native_u128(A) + 1);
  OUT((T), native_u128(A) + 1);
  T = A;
  OUT((--T), native_u128(A) - 1);
  OUT((T), native_u128(A) - 1);

  out << "}\n";
}

#endif /* DO_GENERATE_BASIC */

//------------------------------------------------------------------------------

static void probe(const erthink::uint128_t &a, const erthink::uint128_t &b) {
  ++N;
  ASSERT_EQ((a > b), (native_u128(a) > native_u128(b)));
  ASSERT_EQ((a >= b), (native_u128(a) >= native_u128(b)));
  ASSERT_EQ((a == b), (native_u128(a) == native_u128(b)));
  ASSERT_EQ((a != b), (native_u128(a) != native_u128(b)));
  ASSERT_EQ((a < b), (native_u128(a) < native_u128(b)));
  ASSERT_EQ((a <= b), (native_u128(a) <= native_u128(b)));

  ASSERT_EQ(native_u128(a + b), native_u128(a) + native_u128(b));
  ASSERT_EQ(native_u128(a - b), native_u128(a) - native_u128(b));
  ASSERT_EQ(native_u128(a ^ b), native_u128(a) ^ native_u128(b));
  ASSERT_EQ(native_u128(a | b), native_u128(a) | native_u128(b));
  ASSERT_EQ(native_u128(a & b), native_u128(a) & native_u128(b));
  ASSERT_EQ(native_u128(a * b), native_u128(a) * native_u128(b));

  ASSERT_EQ(native_u128(-a), -native_u128(a));
  ASSERT_EQ(native_u128(~a), ~native_u128(a));
  ASSERT_EQ(!native_u128(!a), !!native_u128(a));

  if (b) {
    const auto pair = erthink::uint128_t::divmod(a, b);
    const auto q = native_u128(a) / native_u128(b);
    const auto r = native_u128(a) % native_u128(b);
    ASSERT_EQ(native_u128(pair.first), q);
    ASSERT_EQ(native_u128(pair.second), r);
  }

  const auto s = unsigned(b) & 127;
  ASSERT_EQ(native_u128(a >> s), native_u128(a) >> s);
  ASSERT_EQ(native_u128(a << s), native_u128(a) << s);
}

static void probe_full(const erthink::uint128_t &a,
                       const erthink::uint128_t &b) {
  ASSERT_NE(native_u128(a), native_u128(a) + 1);
  ASSERT_NE(native_u128(b), native_u128(b) - 1);
  probe(a, b);

  auto t = a;
  ASSERT_EQ(native_u128(t += b), native_u128(a) + native_u128(b));
  t = a;
  ASSERT_EQ(native_u128(t -= b), native_u128(a) - native_u128(b));
  t = a;
  ASSERT_EQ(native_u128(t ^= b), native_u128(a) ^ native_u128(b));
  t = a;
  ASSERT_EQ(native_u128(t |= b), native_u128(a) | native_u128(b));
  t = a;
  ASSERT_EQ(native_u128(t &= b), native_u128(a) & native_u128(b));
  t = a;
  ASSERT_EQ(native_u128(t *= b), native_u128(a) * native_u128(b));

  if (b) {
    t = a;
    ASSERT_EQ(native_u128(t /= b), native_u128(a) / native_u128(b));
    t = a;
    ASSERT_EQ(native_u128(t %= b), native_u128(a) % native_u128(b));
  }

  const auto s = unsigned(b) & 127;
  t = a;
  ASSERT_EQ(native_u128(t >>= s), native_u128(a) >> s);
  t = a;
  ASSERT_EQ(native_u128(t <<= s), native_u128(a) << s);

  ASSERT_EQ(native_u128(ror(a, s)), erthink::ror(native_u128(a), s));
  ASSERT_EQ(native_u128(rol(a, s)), erthink::rol(native_u128(a), s));

  t = a;
  ASSERT_EQ(native_u128(t++), native_u128(a));
  ASSERT_EQ(native_u128(t), native_u128(a) + 1);
  t = a;
  ASSERT_EQ(native_u128(t--), native_u128(a));
  ASSERT_EQ(native_u128(t), native_u128(a) - 1);
  t = a;
  ASSERT_EQ(native_u128(++t), native_u128(a) + 1);
  ASSERT_EQ(native_u128(t), native_u128(a) + 1);
  t = a;
  ASSERT_EQ(native_u128(--t), native_u128(a) - 1);
  ASSERT_EQ(native_u128(t), native_u128(a) - 1);
}

#endif /* ERTHINK_NATIVE_U128_TYPE */

//------------------------------------------------------------------------------

TEST(u128, smoke) {
#ifndef ERTHINK_NATIVE_U128_TYPE
  GTEST_SKIP() << "SKIPPEND because of no native __uint128_t";
#else
  probe_full(0, 0);
  probe_full(~native_u128(0), ~native_u128(0));
  probe_full(~native_u128(0), 11);
  probe_full(7, ~native_u128(0));
  probe_full(1, 0);
  probe_full(0, -2);
  probe_full(3, 42);
  probe_full(~0, 421);
  probe_full(~42, 5);
  probe_full(~421, INT_MAX);
  probe_full(UINT64_C(13632396072180810313), UINT64_C(4895412794877399892));
  probe_full(UINT64_C(5008002785836588600), UINT64_C(6364136223846793005));

  double a = DBL_MAX, b = DBL_MAX;
  while (a + b > 1) {
    a /= 1.1283791670955125739 /* 2/sqrt(pi) */;
    probe_full(native_u128(std::fmod(a, std::ldexp(1.0, 128))),
               native_u128(std::fmod(b, std::ldexp(1.0, 128))));
    probe_full(native_u128(std::fmod(b, std::ldexp(1.0, 128))),
               native_u128(std::fmod(a, std::ldexp(1.0, 128))));
    b *= 0.91893853320467274178 /* ln(sqrt(2pi)) */;
    probe_full(native_u128(std::fmod(a, std::ldexp(1.0, 128))),
               native_u128(std::fmod(b, std::ldexp(1.0, 128))));
    probe_full(native_u128(std::fmod(b, std::ldexp(1.0, 128))),
               native_u128(std::fmod(a, std::ldexp(1.0, 128))));
  }
#endif /* ERTHINK_NATIVE_U128_TYPE */
}

//------------------------------------------------------------------------------

TEST(u128, random3e6) {
#ifndef ERTHINK_NATIVE_U128_TYPE
  GTEST_SKIP() << "SKIPPEND because of no native __uint128_t";
#else
  SCOPED_TRACE("PRNG seed=" + std::to_string(lcg.state));
  for (auto i = 0; i < 33333; ++i) {
    probe_full(lcg(), lcg());
    probe_full({lcg(), lcg()}, lcg());
    probe_full(lcg(), {lcg(), lcg()});
    probe_full({lcg(), lcg()}, {lcg(), lcg()});
    probe_full({lcg(), lcg()}, {lcg(), lcg()});

    probe_full({lcg(), 0}, {lcg(), lcg()});
    probe_full({lcg(), lcg()}, {lcg(), 0});
    probe_full({lcg(), 0}, {lcg(), 0});
    probe_full({lcg(), 0}, lcg());
    probe_full(lcg(), {lcg(), 0});

    probe_full({UINT64_MAX, lcg()}, {lcg(), lcg()});
    probe_full({lcg(), lcg()}, {UINT64_MAX, lcg()});
    probe_full({UINT64_MAX, lcg()}, {UINT64_MAX, lcg()});
    probe_full({UINT64_MAX, lcg()}, lcg());
    probe_full(lcg(), {UINT64_MAX, lcg()});

    if (GTEST_IS_EXECUTION_TIMEOUT())
      break;
  }
#endif /* ERTHINK_NATIVE_U128_TYPE */
}

//------------------------------------------------------------------------------

TEST(u128, DISABLED_stairwell) {
#ifndef ERTHINK_NATIVE_U128_TYPE
  GTEST_SKIP() << "SKIPPEND because of no native __uint128_t";
#else
  SCOPED_TRACE("PRNG seed=" + std::to_string(lcg.state));
  const auto outer = random_shuffle_0_127();
  const auto inner = random_shuffle_0_127();
  // Total 1`065`418`752 probe() calls
  for (const auto i : outer) {
    const auto base_a = (~native_u128(0)) >> i;
    for (const auto j : inner) {
      const auto base_b = (~native_u128(0)) >> j;
      for (auto offset_a = base_a; offset_a >>= 1;) {
        for (auto offset_b = base_b; offset_b >>= 1;) {
          probe(base_a + offset_a, base_b + offset_b);
          probe(base_a + offset_a, base_b - offset_b);
          probe(base_a - offset_a, base_b + offset_b);
          probe(base_a - offset_a, base_b - offset_b);

          probe(base_a + offset_a, ~base_b + offset_b);
          probe(base_a + offset_a, ~base_b - offset_b);
          probe(base_a - offset_a, ~base_b + offset_b);
          probe(base_a - offset_a, ~base_b - offset_b);

          probe(~base_a + offset_a, base_b + offset_b);
          probe(~base_a + offset_a, base_b - offset_b);
          probe(~base_a - offset_a, base_b + offset_b);
          probe(~base_a - offset_a, base_b - offset_b);

          probe(~base_a + offset_a, ~base_b + offset_b);
          probe(~base_a + offset_a, ~base_b - offset_b);
          probe(~base_a - offset_a, ~base_b + offset_b);
          probe(~base_a - offset_a, ~base_b - offset_b);
        }
        probe(base_a + offset_a, base_b);
        probe(base_a - offset_a, base_b);
        probe(base_a + offset_a, ~base_b);
        probe(base_a - offset_a, ~base_b);
        probe(~base_a + offset_a, base_b);
        probe(~base_a - offset_a, base_b);
        probe(~base_a + offset_a, ~base_b);
        probe(~base_a - offset_a, ~base_b);
      }
      probe(base_a, base_b);
      probe(base_a, ~base_b);
      probe(~base_a, base_b);
      probe(~base_a, ~base_b);
      if (GTEST_IS_EXECUTION_TIMEOUT())
        return;
    }
  }
#endif /* ERTHINK_NATIVE_U128_TYPE */
}

//------------------------------------------------------------------------------

runtime_limiter ci_runtime_limiter;

int main(int argc, char **argv) {

#ifdef DO_GENERATE_BASIC
  std::ofstream out("autogen_u128_basic.h++");
  out << "// The contents of this file are generated automatically. \n";
  out << "// You should not edit it manually.\n";
  for (auto i = 0; i < 5; ++i) {
    gen(out, lcg(), lcg());
    gen(out, {lcg(), lcg()}, lcg());
    gen(out, lcg(), {lcg(), lcg()});
    gen(out, {lcg(), lcg()}, {lcg(), lcg()});
    gen(out, {lcg(), lcg()}, {lcg(), lcg()});

    gen(out, {lcg(), 0}, {lcg(), lcg()});
    gen(out, {lcg(), lcg()}, {lcg(), 0});
    gen(out, {lcg(), 0}, {lcg(), 0});
    gen(out, {lcg(), 0}, lcg());
    gen(out, lcg(), {lcg(), 0});

    gen(out, {UINT64_MAX, lcg()}, {lcg(), lcg()});
    gen(out, {lcg(), lcg()}, {UINT64_MAX, lcg()});
    gen(out, {UINT64_MAX, lcg()}, {UINT64_MAX, lcg()});
    gen(out, {UINT64_MAX, lcg()}, lcg());
    gen(out, lcg(), {UINT64_MAX, lcg()});

    if (i == 0)
      out << "#ifndef ERTHINK_NATIVE_U128_TYPE\n";
  }
  out << "#endif /* !ERTHINK_NATIVE_U128_TYPE */\n";
  out.close();
#endif /* DO_GENERATE_BASIC */

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
