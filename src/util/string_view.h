#pragma once

#include <string>

#if __cplusplus < 201703L
#    include "iterator.h"

namespace std {
template<typename CharT>
class basic_string_view {
 public:
    using traits_type = std::char_traits<CharT>;
    using value_type = CharT;
    using pointer = CharT*;
    using const_pointer = const CharT*;
    using reference = CharT&;
    using const_reference = const CharT&;
    using const_iterator = util::array_iterator<basic_string_view, true>;
    using iterator = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    static const size_t npos = size_t(-1);

    basic_string_view() = default;
    basic_string_view(const CharT* s, size_t count) : begin_(s), size_(count) {}
    basic_string_view(const CharT* s) : begin_(s) {
        for (; *s; ++s, ++size_) {}
    }
    basic_string_view(const basic_string<CharT>& s) : basic_string_view(s.data(), s.size()) {}

    size_t size() const { return size_; }
    size_t length() const { return size_; }
    bool empty() const { return size_ == 0; }

    explicit operator basic_string<CharT>() const { return basic_string<CharT>(begin_, size_); }

    const_iterator begin() const { return const_iterator::from_base(begin_, begin_, begin_ + size_); }
    const_iterator cbegin() const { return const_iterator::from_base(begin_, begin_, begin_ + size_); }

    const_iterator end() const { return const_iterator::from_base(begin_ + size_, begin_, begin_ + size_); }
    const_iterator cend() const { return const_iterator::from_base(begin_ + size_, begin_, begin_ + size_); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator{end()}; }
    const_reverse_iterator crbegin() const { return const_reverse_iterator{end()}; }

    const_reverse_iterator rend() const { return const_reverse_iterator{begin()}; }
    const_reverse_iterator crend() const { return const_reverse_iterator{begin()}; }

    const_reference operator[](size_t pos) const {
        assert(pos < size_);
        return begin_[pos];
    }
    const_reference at(size_t pos) const {
        if (pos >= size_) { throw out_of_range("invalid pos"); }
        return begin_[pos];
    }
    const_reference front() const {
        assert(size_ > 0);
        return begin_[0];
    }
    const_reference back() const {
        assert(size_ > 0);
        return *(begin_ + size_ - 1);
    }
    const_pointer data() const { return begin_; }

    basic_string_view substr(size_t pos, size_t count = npos) const {
        pos = std::min(pos, size_);
        return basic_string_view(begin_ + pos, std::min(count, size_ - pos));
    }

    int compare(basic_string_view s) const;
    size_t find(CharT ch, size_t pos = 0) const;
    size_t find(basic_string_view s, size_t pos = 0) const;
    size_t rfind(CharT ch, size_t pos = npos) const;
    size_t rfind(basic_string_view s, size_t pos = npos) const;

    friend basic_string<CharT>& operator+=(basic_string<CharT>& lhs, basic_string_view rhs) {
        lhs.append(rhs.data(), rhs.size());
        return lhs;
    }

 private:
    const CharT* begin_ = nullptr;
    size_t size_ = 0;
};

template<typename CharT>
int basic_string_view<CharT>::compare(basic_string_view<CharT> s) const {
    int result = traits_type::compare(begin_, s.begin_, std::min(size_, s.size_));
    if (result != 0) {
        return result;
    } else if (size_ < s.size_) {
        return -1;
    } else if (s.size_ < size_) {
        return 1;
    }
    return 0;
}

template<typename CharT>
size_t basic_string_view<CharT>::find(CharT ch, size_t pos) const {
    for (auto p = begin_ + pos; p < begin_ + size_; ++p) {
        if (traits_type::eq(*p, ch)) { return p - begin_; }
    }
    return npos;
}

template<typename CharT>
size_t basic_string_view<CharT>::find(basic_string_view s, size_t pos) const {
    for (auto p = begin_ + pos; p + s.size_ <= begin_ + size_; ++p) {
        if (std::equal(p, p + s.size_, s.begin_, traits_type::eq)) { return p - begin_; }
    }
    return npos;
}

template<typename CharT>
size_t basic_string_view<CharT>::rfind(CharT ch, size_t pos) const {
    for (auto p = begin_ + std::min(pos + 1, size_); p > begin_; --p) {
        if (traits_type::eq(*(p - 1), ch)) { return p - begin_ - 1; }
    }
    return npos;
}

template<typename CharT>
size_t basic_string_view<CharT>::rfind(basic_string_view s, size_t pos) const {
    for (auto p = begin_ + std::min(pos + s.size_, size_); p >= begin_ + s.size_; --p) {
        if (std::equal(p - s.size_, p, s.begin_, traits_type::eq)) { return p - begin_ - s.size_; }
    }
    return npos;
}

template<typename CharT>
bool operator==(basic_string_view<CharT> lhs, basic_string_view<CharT> rhs) {
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}
template<typename CharT>
bool operator!=(basic_string_view<CharT> lhs, basic_string_view<CharT> rhs) {
    return lhs.size() != rhs.size() || lhs.compare(rhs) != 0;
}
template<typename CharT>
bool operator<(basic_string_view<CharT> lhs, basic_string_view<CharT> rhs) {
    return lhs.compare(rhs) < 0;
}
template<typename CharT>
bool operator<=(basic_string_view<CharT> lhs, basic_string_view<CharT> rhs) {
    return lhs.compare(rhs) <= 0;
}
template<typename CharT>
bool operator>(basic_string_view<CharT> lhs, basic_string_view<CharT> rhs) {
    return lhs.compare(rhs) > 0;
}
template<typename CharT>
bool operator>=(basic_string_view<CharT> lhs, basic_string_view<CharT> rhs) {
    return lhs.compare(rhs) >= 0;
}

template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator==(basic_string_view<CharT> lhs, const Ty& rhs) {
    return lhs == basic_string_view<CharT>(rhs);
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator!=(basic_string_view<CharT> lhs, const Ty& rhs) {
    return lhs != basic_string_view<CharT>(rhs);
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator<(basic_string_view<CharT> lhs, const Ty& rhs) {
    return lhs < basic_string_view<CharT>(rhs);
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator<=(basic_string_view<CharT> lhs, const Ty& rhs) {
    return lhs <= basic_string_view<CharT>(rhs);
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator>(basic_string_view<CharT> lhs, const Ty& rhs) {
    return lhs > basic_string_view<CharT>(rhs);
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator>=(basic_string_view<CharT> lhs, const Ty& rhs) {
    return lhs >= basic_string_view<CharT>(rhs);
}

template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator==(const Ty& lhs, basic_string_view<CharT> rhs) {
    return basic_string_view<CharT>(lhs) == rhs;
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator!=(const Ty& lhs, basic_string_view<CharT> rhs) {
    return basic_string_view<CharT>(lhs) != rhs;
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator<(const Ty& lhs, basic_string_view<CharT> rhs) {
    return basic_string_view<CharT>(lhs) < rhs;
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator<=(const Ty& lhs, basic_string_view<CharT> rhs) {
    return basic_string_view<CharT>(lhs) <= rhs;
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator>(const Ty& lhs, basic_string_view<CharT> rhs) {
    return basic_string_view<CharT>(lhs) > rhs;
}
template<typename CharT, typename Ty,
         typename = std::enable_if_t<std::is_convertible<Ty, basic_string_view<CharT>>::value>>
bool operator>=(const Ty& lhs, basic_string_view<CharT> rhs) {
    return basic_string_view<CharT>(lhs) >= rhs;
}

using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;

#    if defined(_MSC_VER)
template<typename CharT>
struct hash<basic_string_view<CharT>> {
    size_t operator()(basic_string_view<CharT> s) const { return _Hash_seq((const unsigned char*)s.data(), s.size()); }
};
#    else   // defined(_MSC_VER)
template<typename CharT>
struct hash<basic_string_view<CharT>> {
    size_t operator()(basic_string_view<CharT> s) const {
        return std::hash<std::basic_string<CharT>>{}(static_cast<std::basic_string<CharT>>(s));
    }
};
#    endif  // defined(_MSC_VER)
}  // namespace std
#endif  // __cplusplus < 201703L
