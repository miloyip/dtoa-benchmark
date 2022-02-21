#include "util/string_cvt.h"

#include <array>

using namespace util;

namespace scvt {

template<typename Ty>
struct fp_traits;

template<>
struct fp_traits<double> {
    enum : unsigned { kTotalBits = 64, kBitsPerMantissa = 52 };
    static const uint64_t kSignBit = 1ull << (kTotalBits - 1);
    static const uint64_t kMantissaMask = (1ull << kBitsPerMantissa) - 1;
    static const uint64_t kExpMask = ~kMantissaMask & ~kSignBit;
    static const int kExpMax = kExpMask >> kBitsPerMantissa;
    static const int kExpBias = kExpMax >> 1;
    static uint64_t to_u64(const double& f) { return *reinterpret_cast<const uint64_t*>(&f); }
    static double from_u64(const uint64_t& u64) { return *reinterpret_cast<const double*>(&u64); }
};

template<>
struct fp_traits<float> {
    enum : unsigned { kTotalBits = 32, kBitsPerMantissa = 23 };
    static const uint64_t kSignBit = 1ull << (kTotalBits - 1);
    static const uint64_t kMantissaMask = (1ull << kBitsPerMantissa) - 1;
    static const uint64_t kExpMask = ~kMantissaMask & ~kSignBit & ((1ull << kTotalBits) - 1);
    static const int kExpMax = kExpMask >> kBitsPerMantissa;
    static const int kExpBias = kExpMax >> 1;
    static uint64_t to_u64(const float& f) { return *reinterpret_cast<const uint32_t*>(&f); }
    static float from_u64(const uint64_t& u64) { return *reinterpret_cast<const float*>(&u64); }
};

struct uint96_t {
    uint64_t hi;
    uint32_t lo;
};

struct uint128_t {
    uint64_t hi;
    uint64_t lo;
};

inline uint64_t lo32(uint64_t x) { return x & 0xffffffff; }

inline uint64_t hi32(uint64_t x) { return x >> 32; }

template<typename Ty>
uint64_t make64(Ty hi, Ty lo) {
    return (static_cast<uint64_t>(hi) << 32) | static_cast<uint64_t>(lo);
}

struct ulog2_table_t {
    std::array<unsigned, 256> n_bit;
    CONSTEXPR ulog2_table_t() : n_bit() {
        for (uint32_t n = 0; n < n_bit.size(); ++n) {
            uint32_t u8 = n;
            n_bit[n] = 0;
            while (u8 >>= 1) { ++n_bit[n]; }
        }
    }
};

#if __cplusplus < 201703L
static const ulog2_table_t g_ulog2_tbl;
#else   // __cplusplus < 201703L
constexpr ulog2_table_t g_ulog2_tbl{};
#endif  // __cplusplus < 201703L

inline unsigned ulog2(uint32_t x) {
    unsigned bias = 0;
    if (x >= 1u << 16) { x >>= 16, bias += 16; }
    if (x >= 1u << 8) { x >>= 8, bias += 8; }
    return bias + g_ulog2_tbl.n_bit[x];
}

inline unsigned ulog2(uint64_t x) {
    if (x >= 1ull << 32) { return 32 + ulog2(static_cast<uint32_t>(hi32(x))); }
    return ulog2(static_cast<uint32_t>(lo32(x)));
}

inline unsigned ulog2(uint128_t x) { return x.hi != 0 ? 64 + ulog2(x.hi) : ulog2(x.lo); }

inline uint128_t shl(uint128_t x, unsigned shift) {
    return uint128_t{(x.hi << shift) | (x.lo >> (64 - shift)), x.lo << shift};
}

inline uint128_t shr(uint128_t x, unsigned shift) {
    return uint128_t{x.hi >> shift, (x.lo >> shift) | (x.hi << (64 - shift))};
}

inline uint96_t mul64x32(uint64_t x, uint32_t y, uint32_t bias = 0) {
    uint64_t lower = lo32(x) * y + bias;
    return uint96_t{hi32(x) * y + hi32(lower), static_cast<uint32_t>(lo32(lower))};
}

inline uint128_t mul64x64(uint64_t x, uint64_t y, uint64_t bias = 0) {
    uint64_t lower = lo32(x) * lo32(y) + lo32(bias), higher = hi32(x) * hi32(y);
    uint64_t mid = lo32(x) * hi32(y) + hi32(bias), mid0 = mid;
    mid += hi32(x) * lo32(y) + hi32(lower);
    if (mid < mid0) { higher += 0x100000000; }
    return uint128_t{higher + hi32(mid), make64(lo32(mid), lo32(lower))};
}

struct fp_m96_t {
    uint64_t m;
    uint32_t m2;
    int exp;
};

template<unsigned MaxWords>
struct large_int {
    enum : unsigned { kMaxWords = MaxWords };
    unsigned count = 0;
    std::array<uint64_t, kMaxWords> x;
    large_int() { x.fill(0); }
    explicit large_int(uint32_t val) : large_int() { count = 1, x[0] = val; }

    bool less(const large_int& rhv) const {
        if (count != rhv.count) { return count < rhv.count; };
        for (unsigned n = count; n > 0; --n) {
            if (x[n - 1] != rhv.x[n - 1]) { return x[n - 1] < rhv.x[n - 1]; }
        }
        return false;
    }

    large_int& subtract(const large_int& rhv) {
        unsigned n = 0;
        uint64_t carry = 0;
        for (; n < rhv.count; ++n) {
            uint64_t tmp = x[n] + carry;
            carry = tmp > x[n] || tmp < rhv.x[n] ? ~0ull : 0ull;
            x[n] = tmp - rhv.x[n];
        }
        for (; carry != 0 && n < kMaxWords; ++n) { carry = x[n]-- == 0 ? ~0ull : 0ull; }
        if (n > count) {
            count = n;
        } else {  // track zero words
            while (count > 0 && x[count - 1] == 0) { --count; }
        }
        return *this;
    }

    large_int& negate() {
        unsigned n = 0;
        uint64_t carry = 0;
        for (; n < count; ++n) {
            x[n] = carry - x[n];
            carry = carry != 0 || x[n] != 0 ? ~0ull : 0ull;
        }
        if (carry != 0 && n < kMaxWords) {
            do { --x[n++]; } while (n < kMaxWords);
        } else {  // track zero words
            while (count > 0 && x[count - 1] == 0) { --count; }
        }
        return *this;
    }

    large_int& multiply(uint32_t val) {
        uint96_t mul{0, 0};
        for (unsigned n = 0; n < count; ++n) {
            mul = mul64x32(x[n], val, static_cast<uint32_t>(hi32(mul.hi)));
            x[n] = make64<uint64_t>(mul.hi, mul.lo);
        }
        if (hi32(mul.hi) != 0) { x[count++] = hi32(mul.hi); }
        return *this;
    }

    large_int& shr1() {
        for (unsigned n = 1; n < count; ++n) { x[n - 1] = (x[n - 1] >> 1) | (x[n] << 63); }
        if ((x[count - 1] >>= 1) == 0) { --count; }
        return *this;
    }

    large_int& invert(unsigned word_limit = kMaxWords) {
        large_int a{*this}, div{*this};
        a.negate();
        div.shr1();
        div.x[kMaxWords - 1] |= 1ull << 63;
        count = kMaxWords;
        for (unsigned n = kMaxWords; n > kMaxWords - word_limit; --n) {
            uint64_t mask = 1ull << 63;
            x[n - 1] = 0;
            do {
                if (!a.less(div)) { a.subtract(div), x[n - 1] |= mask; }
                div.shr1();
            } while (mask >>= 1);
            if (x[n - 1] == 0) { --count; }
        }
        return *this;
    }

    template<unsigned MaxWords2>
    std::pair<large_int<MaxWords2>, int> get_normalized() const {
        large_int<MaxWords2> norm;
        unsigned shift = ulog2(x[count - 1]);
        if (count <= MaxWords2) {
            if (shift > 0) {
                norm.x[MaxWords2 - count] = x[0] << (64 - shift);
                for (unsigned n = 1; n < count; ++n) {
                    norm.x[MaxWords2 - count + n] = (x[n] << (64 - shift)) | (x[n - 1] >> shift);
                }
            } else {
                norm.x[MaxWords2 - count] = 0;
                for (unsigned n = 1; n < count; ++n) { norm.x[MaxWords2 - count + n] = x[n - 1]; }
            }
        } else if (shift > 0) {
            for (unsigned n = count - MaxWords2; n < count; ++n) {
                norm.x[MaxWords2 - count + n] = (x[n] << (64 - shift)) | (x[n - 1] >> shift);
            }
        } else {
            for (unsigned n = count - MaxWords2; n < count; ++n) { norm.x[MaxWords2 - count + n] = x[n - 1]; }
        }
        norm.count = MaxWords2;
        while (norm.count > 0 && norm.x[norm.count - 1] == 0) { --norm.count; }
        return {norm, shift + 64 * (count - 1)};
    }

    fp_m96_t make_fp_m96(int exp) const {
        return fp_m96_t{x[kMaxWords - 1], static_cast<uint32_t>(hi32(x[kMaxWords - 2])), exp};
    }
};

struct pow_table_t {
    enum : int { kPow10Max = 400, kPow2Max = 1100, kPrecLimit = 19 };
    enum : uint64_t { kMaxMantissa10 = 10000000000000000000ull };
    std::array<fp_m96_t, 2 * kPow10Max + 1> coef10to2;
    std::array<int, 2 * kPow2Max + 1> exp2to10;
    std::array<uint64_t, 20> decimal_mul;
    pow_table_t() {
        // 10^N -> 2^M power conversion table
        large_int<24> lrg{10};
        coef10to2[kPow10Max] = fp_m96_t{0, 0, 0};
        for (unsigned n = 0; n < kPow10Max; ++n) {
            auto norm = lrg.get_normalized<4>();
            lrg.multiply(10);
            coef10to2[kPow10Max + n + 1] = norm.first.make_fp_m96(norm.second);
            if (norm.first.count != 0) {
                coef10to2[kPow10Max - n - 1] = norm.first.invert(2).make_fp_m96(-norm.second - 1);
            } else {
                coef10to2[kPow10Max - n - 1] = fp_m96_t{0, 0, -norm.second};
            }
        }

        // 2^N -> 10^M power conversion index table
        for (int exp = -kPow2Max; exp <= kPow2Max; ++exp) {
            auto it = std::lower_bound(coef10to2.begin(), coef10to2.end(), -exp,
                                       [](decltype(*coef10to2.begin()) el, int exp) { return el.exp < exp; });
            exp2to10[kPow2Max + exp] = kPow10Max - static_cast<int>(it - coef10to2.begin());
        }

        // decimal multipliers 10^N, N = 0, 1, 2, ...
        uint64_t mul = 1ull;
        for (uint32_t n = 0; n < decimal_mul.size(); ++n, mul *= 10) { decimal_mul[n] = mul; }
    }
};

static const pow_table_t g_pow_tbl;

struct fp_exp10_format {
    uint64_t mantissa = 0;
    int exp = 0;
};

static const char* starts_with(const char* p, const char* end, std::string_view s) {
    if (static_cast<size_t>(end - p) < s.size()) { return p; }
    for (const char *p1 = p, *p2 = s.data(); p1 < end; ++p1, ++p2) {
        char ch1 = std::tolower(static_cast<unsigned char>(*p1));
        char ch2 = std::tolower(static_cast<unsigned char>(*p2));
        if (ch1 != ch2) { return p; }
    }
    return p + s.size();
}

inline const char* skip_spaces(const char* p, const char* end) {
    while (p < end && std::isspace(static_cast<unsigned char>(*p))) { ++p; }
    return p;
}

template<typename Ty>
char get_dig_and_div(Ty& v) {
    Ty t = v;
    v /= 10;
    return '0' + static_cast<unsigned>(t - 10 * v);
}

// ---- from string to value

template<typename Ty>
const char* to_integer(const char* p, const char* end, Ty& val) {
    const char* p0 = p;
    bool neg = false;
    if (p == end) {
        return p0;
    } else if (*p == '+') {
        ++p;  // skip positive sign
    } else if (*p == '-') {
        ++p, neg = true;  // negative sign
    }
    if (p == end || !std::isdigit(static_cast<unsigned char>(*p))) { return p0; }
    val = static_cast<Ty>(*p++ - '0');
    while (p < end && std::isdigit(static_cast<unsigned char>(*p))) { val = 10 * val + static_cast<Ty>(*p++ - '0'); }
    if (neg) { val = -val; }  // apply sign
    return p;
}

static const char* accum_mantissa(const char* p, const char* end, uint64_t& m, int& exp) {
    for (; p < end && std::isdigit(static_cast<unsigned char>(*p)); ++p) {
        if (m < pow_table_t::kMaxMantissa10 / 10) {  // decimal mantissa can hold up to 19 digits
            m = 10 * m + static_cast<uint64_t>(*p - '0');
        } else {
            ++exp;
        }
    }
    return p;
}

static const char* to_fp_exp10(const char* p, const char* end, fp_exp10_format& fp10) {
    if (p == end) {
        return p;
    } else if (std::isdigit(static_cast<unsigned char>(*p))) {  // integer part
        fp10.mantissa = static_cast<uint64_t>(*p++ - '0');
        p = accum_mantissa(p, end, fp10.mantissa, fp10.exp);
        if (p < end && *p == '.') { ++p; }  // skip decimal point
    } else if (*p == '.' && p + 1 < end && std::isdigit(static_cast<unsigned char>(*(p + 1)))) {
        fp10.mantissa = static_cast<uint64_t>(*(p + 1) - '0');  // tenth
        fp10.exp = -1, p += 2;
    } else {
        return p;
    }
    const char* p1 = accum_mantissa(p, end, fp10.mantissa, fp10.exp);  // fractional part
    fp10.exp -= static_cast<unsigned>(p1 - p);
    if (p1 < end && (*p1 == 'e' || *p1 == 'E')) {  // optional exponent
        int exp_optional = 0;
        if ((p = to_integer(p1 + 1, end, exp_optional)) > p1 + 1) { fp10.exp += exp_optional, p1 = p; }
    }
    return p1;
}

template<typename Ty>
const char* to_float(const char* p, const char* end, Ty& val) {
    enum class fp_spec_t { kFinite = 0, kInf, kNaN } special = fp_spec_t::kFinite;
    fp_exp10_format fp10;
    const char* p0 = p;
    bool neg = false;

    if (p == end) {
        return p0;
    } else if (*p == '+') {
        ++p;  // skip positive sign
    } else if (*p == '-') {
        ++p, neg = true;  // negative sign
    }

    int exp = 0;
    uint64_t mantissa2 = 0;
    const char* p1 = to_fp_exp10(p, end, fp10);
    if (p1 > p) {
        if (fp10.mantissa == 0 || fp10.exp < -pow_table_t::kPow10Max) {  // perfect zero
        } else if (fp10.exp > pow_table_t::kPow10Max) {                  // infinity
            exp = fp_traits<Ty>::kExpMax;
        } else {
            // Convert decimal mantissa to binary :
            // Note: coefficients in `coef10to2` are normalized and belong [1, 2) range
            // Note: multiplication of 64-bit mantissa by 96-bit coefficient gives 128+32-bit result:
            // `res128.hi` (higher 64-bit), `res128.lo` (middle 64-bit), and `res96.lo` (lower 32-bit)
            const auto& coef = g_pow_tbl.coef10to2[pow_table_t::kPow10Max + fp10.exp];
            auto res96 = mul64x32(fp10.mantissa, coef.m2);
            auto res128 = mul64x64(fp10.mantissa, coef.m, res96.hi);
            res128.hi += fp10.mantissa;  // apply implicit 1 term of normalized coefficient

            // Obtain binary exponent :
            // Note: res128.hi != 0
            // Note: overflow is possible while summing `res128.hi += fp10.mantissa`, it's normal: log = 64
            const unsigned log = res128.hi < fp10.mantissa ? 64 : ulog2(res128.hi);  // most significant `1` position
            exp = fp_traits<Ty>::kExpBias + log + coef.exp;                          // `exp` is biased binary exponent
            if (exp >= fp_traits<Ty>::kExpMax) {
                exp = fp_traits<Ty>::kExpMax;  // infinity
            } else if (exp <= -static_cast<int>(fp_traits<Ty>::kBitsPerMantissa)) {
                // corner cases: perfect zero or the smallest possible floating point number
                if (exp == -static_cast<int>(fp_traits<Ty>::kBitsPerMantissa)) { mantissa2 = 1ull; }
                exp = 0;
            } else {
                // General case: obtain rounded binary mantissa bits

                // As far as we are going to round, it's convenient to align binary
                // mantissa with left 128-bit boundary of `res128`

                // Move mantissa to the left position so the most significant `1` is hidden
                if (log == 0) {
                    res128.hi = res128.lo;
                    res128.lo = make64<uint64_t>(res96.lo, 0);
                } else if (log < 64) {
                    res128 = shl(res128, 64 - log);
                    res128.lo |= make64<uint64_t>(res96.lo, 0) >> log;
                }

                // When `exp <= 0` mantissa will be denormalized further, so store the real mantissa length
                const unsigned n_bits = exp > 0 ? fp_traits<Ty>::kBitsPerMantissa :
                                                  fp_traits<Ty>::kBitsPerMantissa + exp - 1;

                // Do banker's or `nearest even` rounding :
                // It seems that perfect rounding is not possible, because theoretical binary mantissa
                // representation can be of infinite length. But we use some heuristic to detect exact halves.
                // If we take long enough range of bits after the point where we need to break series then
                // analyzing this bits we can make the decision in which direction to round.
                // The following bits: x x x x x x 1 0 0 0 0 0 0 0 0 0 . . . . . . we consider as exact half
                //                     x x x x x x 0 1 1 1 1 1 1 1 1 1 . . . . . . and this is exact half too
                //                    | needed    | to be rounded     | unknown   |
                // In our case we need `n_bits` left bits in `res128.hi`, all other bits are rounded and dropped.
                // To decide in which direction to round we use reliable bits after `n_bits`. Totally 96 bits are
                // reliable, because `coef10to2` has 96-bit precision. So, we use only `res128.hi` and higher 32-bit
                // part of `res128.lo`.

                // Drop unreliable bits and resolve the case of periodical `1`
                // Note: we do not need to reset lower 32-bit part of `res128.lo`, because it is ignored further
                const uint64_t before_rounding = res128.hi;
                const uint64_t lsb = res128.lo & (1ull << 32);
                res128.lo += lsb;
                if (res128.lo < lsb) { ++res128.hi; }  // handle lower part overflow

                // Do banker's rounding
                const uint64_t half = 1ull << (63 - n_bits);
                res128.hi += hi32(res128.lo) == 0 && (res128.hi & (half << 1)) == 0 ? half - 1 : half;
                if (res128.hi < before_rounding) {  // overflow while rounding
                    // Note: the value can become normalized if `exp == 0`
                    // or infinity if `exp == fp_traits<Ty>::kExpMax - 1`
                    // Note: in case of overflow mantissa will be `0`
                    ++exp;
                }

                // shift mantissa to the right position
                mantissa2 = res128.hi >> (64 - fp_traits<Ty>::kBitsPerMantissa);
                if (exp <= 0) {  // denormalize
                    mantissa2 |= 1ull << fp_traits<Ty>::kBitsPerMantissa;
                    mantissa2 >>= 1 - exp;
                    exp = 0;
                }
            }
        }
    } else if ((p1 = starts_with(p, end, "inf")) > p) {  // infinity
        exp = fp_traits<Ty>::kExpMax;
    } else if ((p1 = starts_with(p, end, "nan")) > p) {  // NaN
        exp = fp_traits<Ty>::kExpMax;
        mantissa2 = fp_traits<Ty>::kMantissaMask;
    } else {
        return p0;
    }

    // Compose floating point value
    val = fp_traits<Ty>::from_u64((neg ? fp_traits<Ty>::kSignBit : 0) |
                                  (static_cast<uint64_t>(exp) << fp_traits<Ty>::kBitsPerMantissa) | mantissa2);
    return p1;
}  // namespace scvt

// ---- from value to string

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_unsigned<Ty>::value>>
Appender fmt_bin(Ty val, const fmt_state& fmt, Appender appender) {
    std::array<char, 65> buf;
    char *last = buf.data() + buf.size(), *p = last;
    if (!!(fmt.flags & fmt_flags::kShowBase)) { *--p = !(fmt.flags & fmt_flags::kUpperCase) ? 'b' : 'B'; }
    do {
        *--p = '0' + static_cast<unsigned>(val & 0x1);
        val >>= 1;
    } while (val != 0);
    unsigned len = static_cast<unsigned>(last - p);
    if (fmt.width > len) {
        if (!(fmt.flags & fmt_flags::kLeadingZeroes)) { return detail::fmt_adjusted(p, last, fmt, appender); }
        appender('0', fmt.width - len);
    }
    appender(p, last);
    return appender;
}

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_unsigned<Ty>::value>>
Appender fmt_oct(Ty val, const fmt_state& fmt, Appender appender) {
    std::array<char, 23> buf;
    char *last = buf.data() + buf.size(), *p = last;
    do {
        *--p = '0' + static_cast<unsigned>(val & 0x7);
        val >>= 3;
    } while (val != 0);
    if (!!(fmt.flags & fmt_flags::kShowBase)) { *--p = '0'; }
    unsigned len = static_cast<unsigned>(last - p);
    if (fmt.width > len) {
        if (!(fmt.flags & fmt_flags::kLeadingZeroes)) { return detail::fmt_adjusted(p, last, fmt, appender); }
        appender('0', fmt.width - len);
    }
    appender(p, last);
    return appender;
}

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_unsigned<Ty>::value>>
Appender fmt_hex(Ty val, const fmt_state& fmt, Appender appender) {
    std::array<char, 18> buf;
    char *last = buf.data() + buf.size(), *p = last;
    const char* digs = !(fmt.flags & fmt_flags::kUpperCase) ? "0123456789abcdef" : "0123456789ABCDEF";
    do {
        *--p = digs[val & 0xf];
        val >>= 4;
    } while (val != 0);
    unsigned len = static_cast<unsigned>(last - p);
    if (!!(fmt.flags & fmt_flags::kShowBase)) {
        len += 2;
        if (fmt.width > len && !!(fmt.flags & fmt_flags::kLeadingZeroes)) {
            appender('0');
            appender(!(fmt.flags & fmt_flags::kUpperCase) ? 'x' : 'X');
            appender('0', fmt.width - len);
        } else {
            p -= 2;
            p[0] = '0', p[1] = !(fmt.flags & fmt_flags::kUpperCase) ? 'x' : 'X';
            if (fmt.width > len) { return detail::fmt_adjusted(p, last, fmt, appender); }
        }
    } else if (fmt.width > len) {
        if (!(fmt.flags & fmt_flags::kLeadingZeroes)) { return detail::fmt_adjusted(p, last, fmt, appender); }
        appender('0', fmt.width - len);
    }
    appender(p, last);
    return appender;
}

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_unsigned<Ty>::value>>
Appender fmt_dec_unsigned(Ty val, const fmt_state& fmt, Appender appender) {
    std::array<char, 20> buf;
    char *last = buf.data() + buf.size(), *p = last;
    do { *--p = get_dig_and_div(val); } while (val != 0);
    unsigned len = static_cast<unsigned>(last - p);
    if (fmt.width > len) {
        if (!(fmt.flags & fmt_flags::kLeadingZeroes)) { return detail::fmt_adjusted(p, last, fmt, appender); }
        appender('0', fmt.width - len);
    }
    appender(p, last);
    return appender;
}

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_signed<Ty>::value>>
Appender fmt_dec_signed(Ty val, const fmt_state& fmt, Appender appender) {
    char sign = '+', show_sign = 0;
    if (val < 0) {
        sign = '-', show_sign = 1, val = -val;  // negative value
    } else if ((fmt.flags & fmt_flags::kSignField) == fmt_flags::kSignPos) {
        show_sign = 1;
    } else if ((fmt.flags & fmt_flags::kSignField) == fmt_flags::kSignAlign) {
        show_sign = 1, sign = ' ';
    }

    std::array<char, 21> buf;
    char *last = buf.data() + buf.size(), *p = last;
    auto uval = static_cast<typename std::make_unsigned<Ty>::type>(val);
    do { *--p = get_dig_and_div(uval); } while (uval != 0);
    unsigned len = static_cast<unsigned>(last - p) + show_sign;
    if (fmt.width > len && !!(fmt.flags & fmt_flags::kLeadingZeroes)) {
        if (show_sign) { appender(sign); }
        appender('0', fmt.width - len);
    } else {
        if (show_sign) { *--p = sign; }
        if (fmt.width > len) { return detail::fmt_adjusted(p, last, fmt, appender); }
    }
    appender(p, last);
    return appender;
}

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_unsigned<Ty>::value>>
Appender fmt_unsigned(Ty val, const fmt_state& fmt, Appender appender) {
    switch (fmt.flags & fmt_flags::kBaseField) {
        case fmt_flags::kBin: return fmt_bin(val, fmt, appender); break;
        case fmt_flags::kOct: return fmt_oct(val, fmt, appender); break;
        case fmt_flags::kHex: return fmt_hex(val, fmt, appender); break;
        default: return fmt_dec_unsigned(val, fmt, appender); break;
    }
    return appender;
}

template<typename Ty, typename Appender, typename = std::enable_if_t<std::is_signed<Ty>::value>>
Appender fmt_signed(Ty val, const fmt_state& fmt, Appender appender) {
    switch (fmt.flags & fmt_flags::kBaseField) {
        case fmt_flags::kBin: {
            return fmt_bin(static_cast<typename std::make_unsigned<Ty>::type>(val), fmt, appender);
        } break;
        case fmt_flags::kOct: {
            return fmt_oct(static_cast<typename std::make_unsigned<Ty>::type>(val), fmt, appender);
        } break;
        case fmt_flags::kHex: {
            return fmt_hex(static_cast<typename std::make_unsigned<Ty>::type>(val), fmt, appender);
        } break;
        default: return fmt_dec_signed(val, fmt, appender); break;
    }
    return appender;
}

static char* fmt_fp_exp10(char* p, const fp_exp10_format& fp10, fmt_flags flags, int prec) {
    bool trim_zeroes = false;
    fmt_flags fp_fmt = flags & fmt_flags::kFloatField;
    if (fp_fmt == fmt_flags::kGeneral) {
        trim_zeroes = true;
        prec = std::max(prec - 1, 0);
        if (fp10.exp >= -4 && fp10.exp <= prec) { fp_fmt = fmt_flags::kFixed, prec -= fp10.exp; }
    }

    uint64_t m = fp10.mantissa;
    int n_zeroes = prec;
    if (m != 0) {
        int n_digs = 1 + prec;
        if (fp_fmt == fmt_flags::kFixed) { n_digs += fp10.exp; }
        // count trailing zeroes
        n_zeroes = std::max(n_digs - pow_table_t::kPrecLimit, 0);
        for (;;) {
            uint64_t t = m / 10;
            if (m > 10 * t) { break; }
            ++n_zeroes, m = t;
        }
    }

    if (fp_fmt != fmt_flags::kFixed) {
        // add exponent
        int exp10 = fp10.exp;
        if (exp10 < 0) { exp10 = -exp10; }
        if (exp10 >= 10) {
            do { *--p = get_dig_and_div(exp10); } while (exp10 != 0);
        } else {
            *--p = '0' + exp10;
            *--p = '0';
        }
        *--p = fp10.exp < 0 ? '-' : '+';
        *--p = !(flags & fmt_flags::kUpperCase) ? 'e' : 'E';

        // fractional part
        char* p0 = p;
        prec -= n_zeroes;
        if (!trim_zeroes) {
            for (; n_zeroes > 0; --n_zeroes) { *--p = '0'; }
        }
        for (; prec > 0; --prec) { *--p = get_dig_and_div(m); }

        // integer part
        if (p < p0 || !!(flags & fmt_flags::kShowPoint)) { *--p = '.'; }
        *--p = '0' + m;
        return p;
    }

    // fractional part
    char* p0 = p;
    if (trim_zeroes) {
        if (n_zeroes < prec) {
            prec -= n_zeroes;
            n_zeroes = 0;
            for (; prec > 0; --prec) { *--p = get_dig_and_div(m); }
        } else {
            n_zeroes -= prec;
        }
    } else if (n_zeroes < prec) {
        prec -= n_zeroes;
        for (; n_zeroes > 0; --n_zeroes) { *--p = '0'; }
        for (; prec > 0; --prec) { *--p = get_dig_and_div(m); }
    } else {
        n_zeroes -= prec;
        for (; prec > 0; --prec) { *--p = '0'; }
    }

    // integer part
    if (p < p0 || !!(flags & fmt_flags::kShowPoint)) { *--p = '.'; }
    for (; n_zeroes > 0; --n_zeroes) { *--p = '0'; }
    do { *--p = get_dig_and_div(m); } while (m != 0);
    return p;
}

template<typename Ty, typename Appender>
Appender fmt_float(Ty val, const fmt_state& fmt, Appender appender) {
    std::array<char, 512> buf;
    char *last = buf.data() + buf.size(), *p = last;

    uint64_t mantissa = fp_traits<Ty>::to_u64(val);
    char sign = '+', show_sign = 0;
    if (mantissa & fp_traits<Ty>::kSignBit) {
        sign = '-', show_sign = 1;  // negative value
    } else if ((fmt.flags & fmt_flags::kSignField) == fmt_flags::kSignPos) {
        show_sign = 1;
    } else if ((fmt.flags & fmt_flags::kSignField) == fmt_flags::kSignAlign) {
        show_sign = 1, sign = ' ';
    }

    // Binary exponent and mantissa
    int exp = static_cast<int>((mantissa & fp_traits<Ty>::kExpMask) >> fp_traits<Ty>::kBitsPerMantissa) -
              fp_traits<Ty>::kExpBias;
    mantissa &= fp_traits<Ty>::kMantissaMask;

    if (fp_traits<Ty>::kExpBias + exp == fp_traits<Ty>::kExpMax) {
        p -= 3;
        if (!(fmt.flags & fmt_flags::kUpperCase)) {
            if (mantissa == 0) {  // infinity
                p[0] = 'i', p[1] = 'n', p[2] = 'f';
            } else {  // NaN
                p[0] = 'n', p[1] = 'a', p[2] = 'n';
            }
        } else {
            if (mantissa == 0) {  // infinity
                p[0] = 'I', p[1] = 'N', p[2] = 'F';
            } else {  // NaN
                p[0] = 'N', p[1] = 'A', p[2] = 'N';
            }
        }
    } else {
        fp_exp10_format fp10;
        int prec = fmt.prec;
        if (prec < 0) { prec = 6; }

        if (exp > -fp_traits<Ty>::kExpBias || mantissa != 0) {
            // Shift binary mantissa so the MSB bit is `1`
            if (exp == -fp_traits<Ty>::kExpBias) {  // handle denormalized form
                const unsigned log = ulog2(mantissa);
                mantissa <<= 63 - log;
                exp -= fp_traits<Ty>::kBitsPerMantissa - log - 1;
            } else {
                mantissa <<= 63 - fp_traits<Ty>::kBitsPerMantissa;
                mantissa |= 1ull << 63;
            }

            // Obtain decimal power
            fp10.exp = g_pow_tbl.exp2to10[pow_table_t::kPow2Max + exp];

            // Evaluate desired decimal mantissa length (its integer part)
            // Note: integer part of decimal mantissa is the digits to output,
            // fractional part of decimal mantissa is to be rounded and dropped
            int n_digs = 1 + prec;  // total digit count for scientific format
            const fmt_flags fp_fmt = fmt.flags & fmt_flags::kFloatField;
            if (fp_fmt == fmt_flags::kFixed) {
                n_digs += fp10.exp;  // for fixed format
            } else if (fp_fmt == fmt_flags::kGeneral && n_digs > 1) {
                --n_digs;  // for general format
            }

            if (n_digs >= 0) {
                n_digs = std::min<int>(n_digs, pow_table_t::kPrecLimit);

                // Calculate decimal mantissa representation :
                // Note: coefficients in `coef10to2` are normalized and belong [1, 2) range
                // Note: multiplication of 64-bit mantissa by 96-bit coefficient gives 128+96-bit result
                // To get the desired count of digits we move up the `coef10to2` table, it's equivalent to
                // multiplying the result by a certain power of 10
                const auto& coef = g_pow_tbl.coef10to2[pow_table_t::kPow10Max - fp10.exp + n_digs - 1];
                auto res128 = mul64x64(mantissa, coef.m, mul64x32(mantissa, coef.m2).hi);
                res128.hi += mantissa;  // apply implicit 1 term of normalized coefficient
                exp += coef.exp;        // sum exponents as well

                // Do banker's or `nearest even` rounding :
                // It seems that perfect rounding is not possible, because theoretical decimal mantissa
                // representation can be of infinite length. But we use some heuristic to detect exact halves.
                // If we take long enough range of bits after the point where we need to break series then
                // analyzing this bits we can make the decision in which direction to round.
                // The following bits: x x x x x x 1 0 0 0 0 0 0 0 0 0 . . . . . . we concider as exact half
                //                     x x x x x x 0 1 1 1 1 1 1 1 1 1 . . . . . . and this is exact half too
                //                    | needed    | to be truncated   | unknown   |
                // In our case we need known count of left bits `res128.hi`, which represent integer part of
                // decimal mantissa, all other bits are rounded and dropped. To decide in which direction to round
                // we use reliable bits after decimal mantissa. Totally 96 bits are reliable, because `coef10to2`
                // has 96-bit precision. So, we use only `res128.hi` and higher 32-bit part of `res128.lo`.

                // Drop unreliable bits and resolve the case of periodical `1`
                const uint64_t lsb = res128.lo & (1ull << 32);
                res128.lo += lsb;
                if (res128.lo < lsb) { ++res128.hi; }  // handle lower part overflow
                res128.lo &= ~((1ull << 32) - 1);      // zero lower 32-bit part of `res128.lo`

                // Overflow is possible while summing `res128.hi += mantissa` or rounding - store this
                // overflowed bit. We must take it into account in further calculations
                const uint64_t higher_bit = res128.hi < mantissa ? 1ull : 0ull;

                // Store the shift needed to align integer part of decimal mantissa with 64-bit boundary
                const unsigned shift = 63 - exp;

                // Note: resulting decimal mantissa has the form `C * 10^n_digs`, where `C` belongs [1, 20) range.
                // So, if `C >= 10` we get decimal mantissa with one excess digit. We should detect these cases
                // and divide decimal mantissa by 10 for scientific format

                if (shift == 0 && higher_bit) {
                    // Special case: decimal mantissa has 65 significant bits. We have one excess digit for sure.
                    // Moreover we can't handle more than 64 bits while outputting the number.
                    ++fp10.exp;
                    // Here we are going to divide 65-bit value by 10. We know the division result for 10^64 summand,
                    // so we just use it. We need to drop one lower digit, so we detect even count of tens.
                    const uint64_t div64 = 1844674407370955161, mod64 = 6;
                    fp10.mantissa = div64 + (fp10.mantissa + mod64) / 10;
                    uint64_t mod = res128.hi - 10 * fp10.mantissa + 5;
                    if (res128.lo == 0 && (fp10.mantissa & 1) == 0) { --mod; }
                    if (mod >= 10) { ++fp10.mantissa; }
                } else {
                    // Align fractional part of with 64-bit boundary
                    uint64_t lower = 0;  // we will store lower shifted bits here
                    if (shift > 0) {
                        if (shift < 64) {
                            lower = res128.lo << (64 - shift);
                            res128 = shr(res128, shift);
                            res128.hi |= higher_bit << (64 - shift);
                        } else if (shift > 64) {
                            lower = res128.hi << (128 - shift);
                            res128.lo = (res128.hi >> (shift - 64)) | (higher_bit << (64 - shift));
                            res128.hi = higher_bit >> (shift - 64);
                        } else {
                            lower = res128.lo;
                            res128.lo = res128.hi;
                            res128.hi = higher_bit;
                        }
                    }

                    if (res128.hi >= g_pow_tbl.decimal_mul[n_digs] &&
                        (fp_fmt != fmt_flags::kFixed || n_digs == pow_table_t::kPrecLimit)) {
                        ++fp10.exp;  // one excess digit
                        // Remove one excess digit for scientific format, do 'nearest even' rounding
                        fp10.mantissa = res128.hi / 10;
                        uint64_t mod = res128.hi - 10 * fp10.mantissa + 5;
                        if (res128.lo == 0 && lower == 0 && (fp10.mantissa & 1) == 0) { --mod; }
                        if (mod >= 10) { ++fp10.mantissa; }
                    } else {  // desired digit count
                        // Do 'nearest even' rounding
                        const uint64_t half = 1ull << 63;
                        const uint64_t frac = lower == 0 && (res128.hi & 1) == 0 ? res128.lo + half - 1 :
                                                                                   res128.lo + half;
                        fp10.mantissa = frac < res128.lo ? res128.hi + 1 : res128.hi;
                        if (fp10.mantissa >= g_pow_tbl.decimal_mul[n_digs]) {
                            ++fp10.exp;  // one excess digit
                            if (fp_fmt != fmt_flags::kFixed || n_digs == pow_table_t::kPrecLimit) {
                                // Remove one excess digit for scientific format
                                // Note: `fp10.mantissa` is exact power of 10 in this case
                                fp10.mantissa /= 10;
                            }
                        }
                    }
                }
            }
        }

        p = fmt_fp_exp10(last, fp10, fmt.flags, prec);

        unsigned len = static_cast<unsigned>(last - p) + show_sign;
        if (fmt.width > len && !!(fmt.flags & fmt_flags::kLeadingZeroes)) {
            if (show_sign) { appender(sign); }
            appender('0', fmt.width - len);
            appender(p, last);
            return appender;
        }
    }

    if (show_sign) { *--p = sign; }
    if (fmt.width > static_cast<unsigned>(last - p)) { return detail::fmt_adjusted(p, last, fmt, appender); }
    appender(p, last);
    return appender;
}

}  // namespace scvt

/*static*/ const char* string_converter<char>::from_string(const char* first, const char* last, char& val) {
    const char* p = scvt::skip_spaces(first, last);
    if (p == last) { return first; }
    val = *p;
    return ++p;
}
/*static*/ std::string string_converter<char>::to_string(std::string&& prefix, char val, const fmt_state& fmt) {
    if (fmt.width > 1) { return detail::fmt_adjusted(&val, &val + 1, fmt, string_appender(prefix)).get(); }
    return std::move(prefix += val);
}
/*static*/ char* string_converter<char>::to_string_to(char* dst, char val, const fmt_state& fmt) {
    if (fmt.width > 1) { return detail::fmt_adjusted(&val, &val + 1, fmt, char_buf_appender(dst)).get(); }
    *dst = val;
    return ++dst;
}
/*static*/ char* string_converter<char>::to_string_to_n(char* dst, size_t n, char val, const fmt_state& fmt) {
    if (fmt.width > 1) { return detail::fmt_adjusted(&val, &val + 1, fmt, char_n_buf_appender(dst, n)).get(); }
    if (n == 0) { return dst; }
    *dst = val;
    return ++dst;
}

/*static*/ const char* string_converter<int8_t>::from_string(const char* first, const char* last, int8_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<int8_t>::to_string(std::string&& prefix, int8_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<int8_t>::to_string_to(char* dst, int8_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<int8_t>::to_string_to_n(char* dst, size_t n, int8_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<int16_t>::from_string(const char* first, const char* last, int16_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<int16_t>::to_string(std::string&& prefix, int16_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<int16_t>::to_string_to(char* dst, int16_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<int16_t>::to_string_to_n(char* dst, size_t n, int16_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<int32_t>::from_string(const char* first, const char* last, int32_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<int32_t>::to_string(std::string&& prefix, int32_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<int32_t>::to_string_to(char* dst, int32_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<int32_t>::to_string_to_n(char* dst, size_t n, int32_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<int64_t>::from_string(const char* first, const char* last, int64_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<int64_t>::to_string(std::string&& prefix, int64_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<int64_t>::to_string_to(char* dst, int64_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<int64_t>::to_string_to_n(char* dst, size_t n, int64_t val, const fmt_state& fmt) {
    return scvt::fmt_signed(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<uint8_t>::from_string(const char* first, const char* last, uint8_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<uint8_t>::to_string(std::string&& prefix, uint8_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<uint8_t>::to_string_to(char* dst, uint8_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<uint8_t>::to_string_to_n(char* dst, size_t n, uint8_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<uint16_t>::from_string(const char* first, const char* last, uint16_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<uint16_t>::to_string(std::string&& prefix, uint16_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<uint16_t>::to_string_to(char* dst, uint16_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<uint16_t>::to_string_to_n(char* dst, size_t n, uint16_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<uint32_t>::from_string(const char* first, const char* last, uint32_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<uint32_t>::to_string(std::string&& prefix, uint32_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<uint32_t>::to_string_to(char* dst, uint32_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<uint32_t>::to_string_to_n(char* dst, size_t n, uint32_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<uint64_t>::from_string(const char* first, const char* last, uint64_t& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_integer(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<uint64_t>::to_string(std::string&& prefix, uint64_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<uint64_t>::to_string_to(char* dst, uint64_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<uint64_t>::to_string_to_n(char* dst, size_t n, uint64_t val, const fmt_state& fmt) {
    return scvt::fmt_unsigned(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<float>::from_string(const char* first, const char* last, float& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_float(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<float>::to_string(std::string&& prefix, float val, const fmt_state& fmt) {
    return scvt::fmt_float(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<float>::to_string_to(char* dst, float val, const fmt_state& fmt) {
    return scvt::fmt_float(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<float>::to_string_to_n(char* dst, size_t n, float val, const fmt_state& fmt) {
    return scvt::fmt_float(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<double>::from_string(const char* first, const char* last, double& val) {
    const char* p = scvt::skip_spaces(first, last);
    last = scvt::to_float(p, last, val);
    return last > p ? last : first;
}
/*static*/ std::string string_converter<double>::to_string(std::string&& prefix, double val, const fmt_state& fmt) {
    return scvt::fmt_float(val, fmt, string_appender(prefix)).get();
}
/*static*/ char* string_converter<double>::to_string_to(char* dst, double val, const fmt_state& fmt) {
    return scvt::fmt_float(val, fmt, char_buf_appender(dst)).get();
}
/*static*/ char* string_converter<double>::to_string_to_n(char* dst, size_t n, double val, const fmt_state& fmt) {
    return scvt::fmt_float(val, fmt, char_n_buf_appender(dst, n)).get();
}

/*static*/ const char* string_converter<bool>::from_string(const char* first, const char* last, bool& val) {
    const char *p = scvt::skip_spaces(first, last), *p0 = p;
    if ((p = scvt::starts_with(p, last, "true")) > p0) {
        val = true;
    } else if ((p = scvt::starts_with(p, last, "false")) > p0) {
        val = false;
    } else if (p < last && std::isdigit(static_cast<unsigned char>(*p))) {
        val = false;
        do {
            if (*p++ != '0') { val = true; }
        } while (p < last && std::isdigit(static_cast<unsigned char>(*p)));
    } else {
        return first;
    }
    return p;
}

/*static*/ std::string string_converter<bool>::to_string(std::string&& prefix, bool val, const fmt_state& fmt) {
    std::string_view sval = !(fmt.flags & fmt_flags::kUpperCase) ? (val ? "true" : "false") : (val ? "TRUE" : "FALSE");
    if (sval.size() < fmt.width) {
        return detail::fmt_adjusted(sval.begin(), sval.end(), fmt, string_appender(prefix)).get();
    }
    return std::move(prefix.append(sval));
}

/*static*/ char* string_converter<bool>::to_string_to(char* dst, bool val, const fmt_state& fmt) {
    std::string_view sval = !(fmt.flags & fmt_flags::kUpperCase) ? (val ? "true" : "false") : (val ? "TRUE" : "FALSE");
    if (sval.size() < fmt.width) {
        return detail::fmt_adjusted(sval.begin(), sval.end(), fmt, char_buf_appender(dst)).get();
    }
    return std::copy(sval.begin(), sval.end(), dst);
}

/*static*/ char* string_converter<bool>::to_string_to_n(char* dst, size_t n, bool val, const fmt_state& fmt) {
    std::string_view sval = !(fmt.flags & fmt_flags::kUpperCase) ? (val ? "true" : "false") : (val ? "TRUE" : "FALSE");
    if (sval.size() < fmt.width) {
        return detail::fmt_adjusted(sval.begin(), sval.end(), fmt, char_n_buf_appender(dst, n)).get();
    }
    return std::copy_n(sval.begin(), std::min<size_t>(sval.size(), n), dst);
}
