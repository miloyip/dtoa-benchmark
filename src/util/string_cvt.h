#pragma once

#include "string_view.h"
#include "utility.h"

#include <algorithm>
#include <cctype>

namespace util {

template<unsigned base>
CONSTEXPR int dig(char ch) {
    return static_cast<int>(ch - '0');
}

template<>
CONSTEXPR int dig<16>(char ch) {
    if (ch >= 'a' && ch <= 'f') { return static_cast<int>(ch - 'a') + 10; }
    if (ch >= 'A' && ch <= 'F') { return static_cast<int>(ch - 'A') + 10; }
    return static_cast<int>(ch - '0');
}

template<typename InputIt, typename InputFn = nofunc>
unsigned from_hex(InputIt in, int digs, InputFn fn = InputFn{}, bool* ok = nullptr) {
    unsigned val = 0;
    while (digs > 0) {
        char ch = fn(*in++);
        val <<= 4;
        --digs;
        if (std::isxdigit(static_cast<unsigned char>(ch))) {
            val |= dig<16>(ch);
        } else {
            if (ok) { *ok = false; }
            return val;
        }
    }
    if (ok) { *ok = true; }
    return val;
}

template<typename OutputIt, typename OutputFn = nofunc>
void to_hex(unsigned val, OutputIt out, int digs, OutputFn fn = OutputFn{}) {
    int shift = digs << 2;
    while (shift > 0) {
        shift -= 4;
        *out++ = fn("0123456789ABCDEF"[(val >> shift) & 0xf]);
    }
}

enum class fmt_flags : unsigned {
    kDec = 0,
    kBin = 1,
    kOct = 2,
    kHex = 3,
    kBaseField = 3,
    kGeneral = 0,
    kFixed = 4,
    kScientific = 8,
    kFloatField = 12,
    kRight = 0,
    kLeft = 0x10,
    kInternal = 0x20,
    kAdjustField = 0x30,
    kLeadingZeroes = 0x40,
    kUpperCase = 0x80,
    kShowBase = 0x100,
    kShowPoint = 0x200,
    kSignNeg = 0,
    kSignPos = 0x400,
    kSignAlign = 0x800,
    kSignField = 0xC00,
};

inline fmt_flags operator&(fmt_flags lhs, fmt_flags rhs) {
    return static_cast<fmt_flags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));
}
inline fmt_flags operator|(fmt_flags lhs, fmt_flags rhs) {
    return static_cast<fmt_flags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}
inline fmt_flags operator~(fmt_flags flags) { return static_cast<fmt_flags>(~static_cast<unsigned>(flags)); }
inline bool operator!(fmt_flags flags) { return static_cast<unsigned>(flags) == 0; }
inline fmt_flags& operator&=(fmt_flags& lhs, fmt_flags rhs) { return lhs = lhs & rhs; }
inline fmt_flags& operator|=(fmt_flags& lhs, fmt_flags rhs) { return lhs = lhs | rhs; }

struct fmt_state {
    CONSTEXPR fmt_state() = default;
    CONSTEXPR fmt_state(fmt_flags fl) : flags(fl) {}
    CONSTEXPR fmt_state(fmt_flags fl, int p) : flags(fl), prec(p) {}
    CONSTEXPR fmt_state(fmt_flags fl, int p, unsigned w, char ch) : flags(fl), prec(p), width(w), fill(ch) {}
    fmt_flags flags = fmt_flags::kDec;
    int prec = -1;
    unsigned width = 0;
    char fill = ' ';
};

template<typename Ty>
struct string_converter;

template<typename Ty>
struct string_converter_base {
    using is_string_converter = int;
    static Ty default_value() { return {}; }
};

template<>
struct UTIL_EXPORT string_converter<char> : string_converter_base<char> {
    static const char* from_string(const char* first, const char* last, char& val);
    static std::string to_string(std::string&& prefix, char val, const fmt_state& fmt);
    static char* to_string_to(char* dst, char val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, char val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<int8_t> : string_converter_base<int8_t> {
    static const char* from_string(const char* first, const char* last, int8_t& val);
    static std::string to_string(std::string&& prefix, int8_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, int8_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, int8_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<int16_t> : string_converter_base<int16_t> {
    static const char* from_string(const char* first, const char* last, int16_t& val);
    static std::string to_string(std::string&& prefix, int16_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, int16_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, int16_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<int32_t> : string_converter_base<int32_t> {
    static const char* from_string(const char* first, const char* last, int32_t& val);
    static std::string to_string(std::string&& prefix, int32_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, int32_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, int32_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<int64_t> : string_converter_base<int64_t> {
    static const char* from_string(const char* first, const char* last, int64_t& val);
    static std::string to_string(std::string&& prefix, int64_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, int64_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, int64_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<uint8_t> : string_converter_base<uint8_t> {
    static const char* from_string(const char* first, const char* last, uint8_t& val);
    static std::string to_string(std::string&& prefix, uint8_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, uint8_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, uint8_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<uint16_t> : string_converter_base<uint16_t> {
    static const char* from_string(const char* first, const char* last, uint16_t& val);
    static std::string to_string(std::string&& prefix, uint16_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, uint16_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, uint16_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<uint32_t> : string_converter_base<uint32_t> {
    static const char* from_string(const char* first, const char* last, uint32_t& val);
    static std::string to_string(std::string&& prefix, uint32_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, uint32_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, uint32_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<uint64_t> : string_converter_base<uint64_t> {
    static const char* from_string(const char* first, const char* last, uint64_t& val);
    static std::string to_string(std::string&& prefix, uint64_t val, const fmt_state& fmt);
    static char* to_string_to(char* dst, uint64_t val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, uint64_t val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<float> : string_converter_base<float> {
    static const char* from_string(const char* first, const char* last, float& val);
    static std::string to_string(std::string&& prefix, float val, const fmt_state& fmt);
    static char* to_string_to(char* dst, float val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, float val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<double> : string_converter_base<double> {
    static const char* from_string(const char* first, const char* last, double& val);
    static std::string to_string(std::string&& prefix, double val, const fmt_state& fmt);
    static char* to_string_to(char* dst, double val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, double val, const fmt_state& fmt);
};

template<>
struct UTIL_EXPORT string_converter<bool> : string_converter_base<bool> {
    static const char* from_string(const char* first, const char* last, bool& val);
    static std::string to_string(std::string&& prefix, bool val, const fmt_state& fmt);
    static char* to_string_to(char* dst, bool val, const fmt_state& fmt);
    static char* to_string_to_n(char* dst, size_t n, bool val, const fmt_state& fmt);
};

template<typename Ty, typename Def>
Ty from_string(std::string_view s, Def&& def) {
    Ty result(std::forward<Def>(def));
    string_converter<Ty>::from_string(s.data(), s.data() + s.size(), result);
    return result;
}

template<typename Ty>
Ty from_string(std::string_view s) {
    Ty result(string_converter<Ty>::default_value());
    string_converter<Ty>::from_string(s.data(), s.data() + s.size(), result);
    return result;
}

template<typename Ty, typename... Args>
std::string to_string(const Ty& val, Args&&... args) {
    return string_converter<Ty>::to_string({}, val, fmt_state(std::forward<Args>(args)...));
}

template<typename Ty>
std::string to_string_fmt(std::string&& prefix, const Ty& val, const fmt_state& fmt) {
    return string_converter<Ty>::to_string(std::move(prefix), val, fmt);
}

template<typename Ty>
char* to_string_fmt_to(char* dst, const Ty& val, const fmt_state& fmt) {
    return string_converter<Ty>::to_string_to(dst, val, fmt);
}

template<typename Ty>
char* to_string_fmt_to_n(char* dst, size_t n, const Ty& val, const fmt_state& fmt) {
    return string_converter<Ty>::to_string_to_n(dst, n, val, fmt);
}

class string_appender {
 public:
    explicit string_appender(std::string& str) : str_(str) {}
    std::string get() { return std::move(str_); }
    template<typename InputIt,
             typename = std::enable_if_t<std::is_base_of<
                 std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value>>
    void operator()(InputIt first, InputIt last) {
        str_.append(first, last);
    }
    void operator()(char ch, size_t count) { str_.append(count, ch); }
    void operator()(char ch) { str_ += ch; }
    template<typename Ty, typename = std::void_t<typename string_converter<Ty>::is_string_converter>>
    void format(const Ty& arg, const fmt_state& fmt) {
        str_ = to_string_fmt(std::move(str_), arg, fmt);
    }

 private:
    std::string& str_;
};

class char_buf_appender {
 public:
    explicit char_buf_appender(char* dst) : dst_(dst) {}
    char* get() { return dst_; }
    template<typename InputIt,
             typename = std::enable_if_t<std::is_base_of<
                 std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value>>
    void operator()(InputIt first, InputIt last) {
        dst_ = std::copy(first, last, dst_);
    }
    void operator()(char ch, size_t count) { dst_ = std::fill_n(dst_, count, ch); }
    void operator()(char ch) { *dst_++ = ch; }
    template<typename Ty, typename = std::void_t<typename string_converter<Ty>::is_string_converter>>
    void format(const Ty& arg, const fmt_state& fmt) {
        dst_ = to_string_fmt_to(dst_, arg, fmt);
    }

 private:
    char* dst_;
};

class char_n_buf_appender {
 public:
    char_n_buf_appender(char* dst, size_t n) : dst_(dst), last_(dst + n) {}
    char* get() { return dst_; }
    template<typename InputIt,
             typename = std::enable_if_t<std::is_base_of<
                 std::random_access_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>::value>>
    void operator()(InputIt first, InputIt last) {
        dst_ = std::copy_n(first, std::min<size_t>(last - first, last_ - dst_), dst_);
    }
    void operator()(char ch, size_t count) { dst_ = std::fill_n(dst_, std::min<size_t>(count, last_ - dst_), ch); }
    void operator()(char ch) {
        if (dst_ != last_) { *dst_++ = ch; }
    }
    template<typename Ty, typename = std::void_t<typename string_converter<Ty>::is_string_converter>>
    void format(const Ty& arg, const fmt_state& fmt) {
        dst_ = to_string_fmt_to_n(dst_, last_ - dst_, arg, fmt);
    }

 private:
    char* dst_;
    char* last_;
};

namespace detail {
template<typename InputIt, typename Appender,
         typename = std::enable_if_t<std::is_base_of<std::random_access_iterator_tag,
                                                     typename std::iterator_traits<InputIt>::iterator_category>::value>>
Appender fmt_adjusted(InputIt first, InputIt last, const fmt_state& fmt, Appender appender) {
    size_t len = static_cast<size_t>(last - first);
    switch (fmt.flags & fmt_flags::kAdjustField) {
        case fmt_flags::kLeft: {
            appender(first, last);
            appender(fmt.fill, fmt.width - len);
        } break;
        case fmt_flags::kInternal: {
            size_t right = static_cast<size_t>(fmt.width) - len, left = right >> 1;
            right -= left;
            appender(fmt.fill, left);
            appender(first, last);
            appender(fmt.fill, right);
        } break;
        default: {
            appender(fmt.fill, fmt.width - len);
            appender(first, last);
        } break;
    }
    return appender;
}
}  // namespace detail

}  // namespace util
