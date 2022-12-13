#ifndef ENGINE_ZIP_HPP
#define ENGINE_ZIP_HPP

#include <algorithm>
#include <zen/meta/tuple.hpp>

namespace zen::view {
    namespace detail {
        template <typename...TT>
        struct zip_iterator {
            explicit zip_iterator(TT...tt) : data { tt... } {}

            constexpr auto
            operator *() {
                return
                [&] <std::size_t...NN>
                (std::index_sequence<NN...>) {
                    return forward_as_tuple(*data[I<NN>]...);
                } (seq<sizeof...(TT)>{});
            }

            constexpr auto
            operator ++() {
                return
                [&] <std::size_t...NN>
                (std::index_sequence<NN...>) {
                    return ((++data[I<NN>]), ...);
                } (seq<sizeof...(TT)>{});
            }

            constexpr auto
            operator == (zip_iterator& that) const {
                return
                [&] <std::size_t...NN>
                (std::index_sequence<NN...>) {
                    if((... && (data[I<NN>] == that.data[I<NN>]))) {
                        return true;
                    }
                    return false;
                } (seq<sizeof...(TT)>{});
            }

            constexpr auto
            operator != (zip_iterator& that) const {
                return
                [&] <std::size_t...NN>
                (std::index_sequence<NN...>) {
                    if((... && (data[I<NN>] != that.data[I<NN>]))) {
                        return true;
                    }
                    return false;
                } (seq<sizeof...(TT)>{});
            }

            tuple<TT...> data;
        };
    }

    template <typename...TT>
    struct zip {
        explicit zip(TT...tt) : tup(tuple<TT&&...>{ std::move(tt)... }) {}

        constexpr auto
        begin() {
            return
            [&] <std::size_t...NN>
            (std::index_sequence<NN...>) {
                return detail::zip_iterator { tup[I<NN>].begin()... };
            } (seq<sizeof...(TT)>{});
        }

        auto end() {
            std::size_t min = std::string::npos;
            [&] <std::size_t...NN>
            (std::index_sequence<NN...>) {
                ((min = (tup[I<NN>].size() < min ? tup[I<NN>].size() : min)), ...);
            } (seq<sizeof...(TT)>{});

            return
            [&] <std::size_t...NN>
            (std::index_sequence<NN...>) {
                detail::zip_iterator z { tup[I<NN>].begin()... };
                ((std::advance(z.data[I<NN>], min)), ...);
                return z;
            } (seq<sizeof...(TT)>{});
        }

    private:
        tuple<TT&&...> tup;
    };
}

#endif //ENGINE_ZIP_HPP
