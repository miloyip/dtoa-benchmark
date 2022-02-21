#pragma once

#include "string_cvt.h"

#include <vector>

namespace util {

struct setw {
    unsigned width;
    explicit setw(unsigned w) : width(w) {}
};

struct setprec {
    int prec;
    explicit setprec(int p) : prec(p) {}
};

namespace detail {

inline const char* parse_fmt_adjustment(const char* p, fmt_state& fmt) {
    switch (*p) {
        case '<': fmt.flags |= fmt_flags::kRight; return p + 1;
        case '^': fmt.flags |= fmt_flags::kInternal; return p + 1;
        case '>': return p + 1;
        default: {
            switch (*(p + 1)) {
                case '<': fmt.fill = *p, fmt.flags |= fmt_flags::kRight; return p + 2;
                case '^': fmt.fill = *p, fmt.flags |= fmt_flags::kInternal; return p + 2;
                case '>': fmt.fill = *p; return p + 2;
                default: break;
            }
        } break;
    }
    return p;
}

inline const char* parse_fmt_sign(const char* p, fmt_state& fmt) {
    switch (*p) {
        case '+': fmt.flags |= fmt_flags::kSignPos; return p + 1;
        case ' ': fmt.flags |= fmt_flags::kSignAlign; return p + 1;
        case '-': return p + 1;
        default: break;
    }
    return p;
}

inline const char* parse_fmt_alternate(const char* p, fmt_state& fmt) {
    if (*p == '#') {
        fmt.flags |= fmt_flags::kShowPoint | fmt_flags::kShowBase;
        return p + 1;
    }
    return p;
}

inline const char* parse_fmt_leading_zeroes(const char* p, fmt_state& fmt) {
    if (*p == '0') {
        fmt.flags |= fmt_flags::kLeadingZeroes;
        return p + 1;
    }
    return p;
}

inline const char* parse_fmt_width(const char* p, fmt_state& fmt) {
    while (std::isdigit(static_cast<unsigned char>(*p))) {
        fmt.width = 10 * fmt.width + static_cast<unsigned>(*p++ - '0');
    }
    return p;
}

inline const char* parse_fmt_precision(const char* p, fmt_state& fmt) {
    if (*p != '.') { return p; }
    if (!std::isdigit(static_cast<unsigned char>(*++p))) { return p - 1; }
    fmt.prec = static_cast<int>(*p++ - '0');
    while (std::isdigit(static_cast<unsigned char>(*p))) { fmt.prec = 10 * fmt.prec + static_cast<int>(*p++ - '0'); }
    return p;
}

inline const char* parse_fmt_type(const char* p, fmt_state& fmt) {
    switch (*p++) {
        case 's': return p;
        case 'c': return p;
        case 'b': fmt.flags |= fmt_flags::kBin; return p;
        case 'B': fmt.flags |= fmt_flags::kBin | fmt_flags::kUpperCase; return p;
        case 'o': fmt.flags |= fmt_flags::kOct; return p;
        case 'd': return p;
        case 'x': fmt.flags |= fmt_flags::kHex; return p;
        case 'X': fmt.flags |= fmt_flags::kHex | fmt_flags::kUpperCase; return p;
        case 'p': return p;
        case 'P': fmt.flags |= fmt_flags::kUpperCase; return p;
        case 'f': fmt.flags |= fmt_flags::kFixed; return p;
        case 'F': fmt.flags |= fmt_flags::kFixed | fmt_flags::kUpperCase; return p;
        case 'e': fmt.flags |= fmt_flags::kScientific; return p;
        case 'E': fmt.flags |= fmt_flags::kScientific | fmt_flags::kUpperCase; return p;
        case 'g': return p;
        case 'G': fmt.flags |= fmt_flags::kUpperCase; return p;
        default: break;
    }
    return p - 1;
}

template<typename Appender>
class fmt_context {
 public:
    template<typename... Args>
    fmt_context(std::string_view fmt, Args&&... args)
        : first_(fmt.data()), last_(fmt.data() + fmt.size()), appender_(std::forward<Args>(args)...) {}
    Appender& get_appender() { return appender_; }
    void set_width(unsigned width) { arg_fmt_.width = width; }
    void set_prec(int prec) { arg_fmt_.prec = prec; }
    bool parse();

    void append(std::string_view s) {
        if (s.size() < arg_fmt_.width) {
            detail::fmt_adjusted(s.begin(), s.end(), arg_fmt_, appender_);
        } else {
            appender_(s.begin(), s.end());
        }
    }
    void append(const char* cstr) { append(std::string_view(cstr)); }
    void append(void* p) {
        arg_fmt_.flags &= ~fmt_flags::kBaseField;
        arg_fmt_.flags |= fmt_flags::kHex | fmt_flags::kShowBase;
        appender_.format(reinterpret_cast<uintptr_t>(p), arg_fmt_);
    }
    template<typename Ty, typename = std::void_t<typename string_converter<Ty>::is_string_converter>>
    void append(const Ty& arg) {
        appender_.format(arg, arg_fmt_);
    }

 private:
    const char* first_;
    const char* last_;
    fmt_state arg_fmt_;
    Appender appender_;
};

template<typename Appender>
bool fmt_context<Appender>::parse() {
    const char* p = first_;
    arg_fmt_ = fmt_state();
    while (first_ != last_) {
        if (*first_ == '{' || *first_ == '}') {
            appender_(p, first_);
            p = ++first_;
            if (first_ == last_) { break; }
            int balance = 1;
            if (*(first_ - 1) == '{' && *first_ != '{') {
                do {
                    if (*first_ == '}' && --balance == 0) {
                        if (*p++ == ':') {
                            p = parse_fmt_adjustment(p, arg_fmt_);
                            p = parse_fmt_sign(p, arg_fmt_);
                            p = parse_fmt_alternate(p, arg_fmt_);
                            p = parse_fmt_leading_zeroes(p, arg_fmt_);
                            p = parse_fmt_width(p, arg_fmt_);
                            p = parse_fmt_precision(p, arg_fmt_);
                            p = parse_fmt_type(p, arg_fmt_);
                        }
                        ++first_;
                        return true;
                    } else if (*first_ == '{') {
                        ++balance;
                    }
                } while (++first_ != last_);
                return false;
            }
        }
        ++first_;
    }
    appender_(p, first_);
    return false;
}

template<typename Context>
void format(Context& ctx) {}

template<typename Context, typename Ty, typename... Ts>
void format(Context& ctx, const Ty& arg, Ts&&... other) {
    ctx.append(arg);
    if (ctx.parse()) { detail::format(ctx, std::forward<Ts>(other)...); }
}

template<typename Context, typename... Ts>
void format(Context& ctx, setw w, Ts&&... other) {
    ctx.set_width(w.width);
    detail::format(ctx, std::forward<Ts>(other)...);
}

template<typename Context, typename... Ts>
void format(Context& ctx, setprec p, Ts&&... other) {
    ctx.set_prec(p.prec);
    detail::format(ctx, std::forward<Ts>(other)...);
}

}  // namespace detail

template<typename... Ts>
std::string format(std::string_view fmt, Ts&&... args) {
    std::string result;
    result.reserve(256);
    detail::fmt_context<string_appender> ctx(fmt, result);
    if (ctx.parse()) { detail::format(ctx, std::forward<Ts>(args)...); }
    return std::move(result);
}

template<typename... Ts>
char* format_to(char* dst, std::string_view fmt, Ts&&... args) {
    detail::fmt_context<char_buf_appender> ctx(fmt, dst);
    if (ctx.parse()) { detail::format(ctx, std::forward<Ts>(args)...); }
    return ctx.get_appender().get();
}

template<typename... Ts>
char* format_to_n(char* dst, size_t n, std::string_view fmt, Ts&&... args) {
    detail::fmt_context<char_n_buf_appender> ctx(fmt, dst, n);
    if (ctx.parse()) { detail::format(ctx, std::forward<Ts>(args)...); }
    return ctx.get_appender().get();
}

}  // namespace util
