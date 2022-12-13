#ifndef ENGINE_XTRAITS_HPP
#define ENGINE_XTRAITS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

namespace zen {
    template <std::size_t N>
    using integral = std::integral_constant<std::size_t, N>;

    template <std::size_t N>
    using seq = decltype(std::make_index_sequence<N>());

    template <std::size_t N>
    constexpr auto I = integral<N>{};

    template <typename T>
    struct type {
        using value = T;
    };

    template <typename...TT>
    struct types {};

    template <typename U>
    constexpr auto T = type<U>{};

    template <typename> struct pointer;

    template <typename R, typename...Args>
    struct pointer<R(*)(Args...)> {
        using ret = R;
        using base = void;
        using args = types<Args...>;
    };

    template <typename R, typename O, typename...Args>
    struct pointer<R(O::*)(Args...)> {
        using ret = R;
        using base = O;
        using args = types<Args...>;
    };

    template <typename R, typename O, typename...Args>
    struct pointer<R(O::*)(Args...) const> {
        using ret = R;
        using base = O;
        using args = types<Args...>;
    };
}

#endif //ADVENT2022_XTRAITS_HPP
