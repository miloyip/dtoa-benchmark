#pragma once

#include "common.h"

#include <type_traits>
#include <utility>

namespace util {
template<typename Ty, typename TestTy = void>
struct type_identity {
    using type = Ty;
};
template<typename Ty, typename TestTy = void>
using type_identity_t = typename type_identity<Ty, TestTy>::type;
}  // namespace util

#if __cplusplus < 201703L
namespace util {
namespace detail {
template<size_t... Indices>
struct index_sequence {};
template<size_t N, size_t... Next>
struct make_index_sequence : make_index_sequence<N - 1U, N - 1U, Next...> {};
template<size_t... Next>
struct make_index_sequence<0U, Next...> {
    using type = index_sequence<Next...>;
};
}  // namespace detail
}  // namespace util
namespace std {
#    if !defined(_MSC_VER) && __cplusplus < 201402L
template<bool B, typename Ty = void>
using enable_if_t = typename enable_if<B, Ty>::type;
template<typename Ty>
using decay_t = typename decay<Ty>::type;
template<typename Ty>
using remove_reference_t = typename remove_reference<Ty>::type;
template<typename Ty>
using remove_const_t = typename remove_const<Ty>::type;
template<typename Ty>
using remove_cv_t = typename remove_cv<Ty>::type;
template<typename Ty>
using add_const_t = typename add_const<Ty>::type;
template<bool B, typename Ty1, typename Ty2>
using conditional_t = typename conditional<B, Ty1, Ty2>::type;
#    endif  // !defined(_MSC_VER) && __cplusplus < 201402L
template<class Ty>
add_const_t<Ty>& as_const(Ty& t) {
    return t;
}
template<class Ty>
void as_const(const Ty&&) = delete;
template<typename TestTy>
using void_t = typename util::type_identity<void, TestTy>::type;
template<bool B>
using bool_constant = integral_constant<bool, B>;
#    if !defined(_MSC_VER)
template<typename Ty>
using is_nothrow_swappable = bool_constant<noexcept(swap(declval<Ty&>(), declval<Ty&>()))>;
#    endif  // !defined(_MSC_VER)
#    if __cplusplus < 201402L
template<size_t... Indices>
using index_sequence = util::detail::index_sequence<Indices...>;
template<size_t N>
using make_index_sequence = typename util::detail::make_index_sequence<N>::type;
template<typename... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;
#    endif  // __cplusplus < 201402L
}  // namespace std
#endif  // __cplusplus < 201703L

namespace util {

template<typename Ty>
struct remove_const : std::remove_const<Ty> {};
template<typename Ty>
using remove_const_t = typename remove_const<Ty>::type;
template<typename Ty1, typename Ty2>
struct remove_const<std::pair<Ty1, Ty2>> {
    using type = std::pair<std::remove_const_t<Ty1>, std::remove_const_t<Ty2>>;
};

template<typename Ty>
Ty get_and_set(Ty& v, std::remove_reference_t<Ty> v_new) {
    std::swap(v, v_new);
    return v_new;
}

struct nofunc {
    template<typename Ty>
    auto operator()(Ty&& v) const -> decltype(std::forward<Ty>(v)) {
        return std::forward<Ty>(v);
    }
};

struct plus {
    template<typename TyL, typename TyR>
    std::decay_t<TyL> operator()(TyL&& lhs, const TyR& rhs) const {
        std::decay_t<TyL> result(std::move(lhs));
        result += rhs;
        return result;
    }
};

}  // namespace util
