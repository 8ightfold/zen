#ifndef ENGINE_CTTYPEID_HPP
#define ENGINE_CTTYPEID_HPP

#include <array>
#include <iostream>
#include <string_view>
#include <type_traits>

#if defined(__clang__)
#  define ZEN_PRETTY_FUNCTION __PRETTY_FUNCTION__
#  define ZEN_PRETTY_FUNCTION_START "[T = "
#  define ZEN_PRETTY_FUNCTION_END "]\0"
#elif defined(__GNUC__) && defined(__GNUG__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#  define ZEN_PRETTY_FUNCTION __PRETTY_FUNCTION__
#  define ZEN_PRETTY_FUNCTION_START "[with T = "
#  define ZEN_PRETTY_FUNCTION_END "]\0"
#elif defined(_MSC_VER)
#  define ZEN_PRETTY_FUNCTION __FUNCSIG__
#  define ZEN_PRETTY_FUNCTION_START "pretty_function_array<"
#  define ZEN_PRETTY_FUNCTION_END ">(void)\0"
#else
#endif

namespace zen {
    namespace detail {

        template <std::size_t...Indices>
        constexpr auto
        substr_to_array(std::string_view str, std::index_sequence<Indices...>) noexcept
        {
            return std::array { str[Indices]..., '\0' };
        };

        template <typename T>
        constexpr auto
        pretty_function_array() noexcept
        {
            constexpr std::string_view name = std::string_view{ ZEN_PRETTY_FUNCTION };
            constexpr std::string_view prefix = { ZEN_PRETTY_FUNCTION_START };
            constexpr std::string_view suffix = { ZEN_PRETTY_FUNCTION_END };

            constexpr std::size_t start = name.find(prefix) + prefix.size();
            constexpr std::size_t end = name.rfind(suffix);

            static_assert(start < end, "Invalid arguments.");

            constexpr std::string_view pname = name.substr(start, (end - start));
            return substr_to_array(pname, std::make_index_sequence<pname.size()>{});
        };


        template <typename T>
        struct prettyparse {
        protected: static inline constexpr auto pretty_name = pretty_function_array<T>();
        };

        template <typename T>
        struct cttypeid_impl : prettyparse<T>
        {
            constexpr cttypeid_impl() : nameof(pretty.data()) {}

            [[nodiscard]]
            constexpr std::string_view
            name() const noexcept { return this->nameof; }

            [[nodiscard]]
            std::string
            string() const noexcept {
                return std::string { this->nameof };
            }

            friend std::ostream&
            operator << (std::ostream& os, const cttypeid_impl& id) {
                return os << id.name();
            }
        private:
            static constexpr auto pretty { prettyparse<T>::pretty_name };
            std::string_view nameof;
        };
    }

    template <typename T>
    constexpr auto cttypeid { detail::cttypeid_impl<T>{} };
}

#endif //ENGINE_CTTYPEID_HPP
