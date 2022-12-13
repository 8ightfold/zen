#ifndef ENGINE_PRINT_CALLER_HPP
#define ENGINE_PRINT_CALLER_HPP

#include <zen/meta/cttypeid.hpp>
#include <string>
#include <string_view>

namespace zen::debug {
    struct caller_parse_exception : std::exception {
        [[nodiscard]]
        const char*
        what() const noexcept override {
            return "current compiler is not valid.";
        }
    };

    namespace detail {
        std::string parse_caller(std::string_view, std::string_view);
    }
    void print_caller(std::string_view, std::string_view);
}

#define ZEN_PRINT_CALLER() zen::debug::print_caller(ZEN_PRETTY_FUNCTION, __func__)
#define $print_caller() zen::debug::print_caller(ZEN_PRETTY_FUNCTION, __func__)

#endif //ENGINE_PRINT_CALLER_HPP
