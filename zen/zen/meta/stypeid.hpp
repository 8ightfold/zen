#ifndef ENGINE_STYPEID_HPP
#define ENGINE_STYPEID_HPP

#include <iostream>
#include <optional>
#include <string>
#include <typeinfo>
#include <utility>

#include "cttypeid.hpp"

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#ifdef ZEN_ENABLE_STYPEID_LOGGING
# include <cstdio>
#  define $stypeid_log(...) printf(__VA_ARGS__)
#else
#  define $stypeid_log(...) do {} while(0)
#endif

namespace zen {
    struct demangler {
    protected:
        static std::optional<std::string>
        demangle(std::string mangled);
    };

#define $CONCAT_BASE_STYPEID(x, y) x##y
#define $CONCAT_STYPEID(x, y) $CONCAT_BASE_STYPEID(x, y)

#define $REPLACE_STYPEID(str, key, value)                                                   \
    [&]() {                                                                                 \
        auto $CONCAT_STYPEID(local_var_, __LINE__) = str.find(key);                         \
        if($CONCAT_STYPEID(local_var_, __LINE__) != decltype(str)::npos) {                  \
            str.replace($CONCAT_STYPEID(local_var_, __LINE__), sizeof(key) - 1, value);     \
            return true;                                                                    \
        }                                                                                   \
        return false;                                                                       \
    }()

#define $REMOVE_STYPEID(str, key) $REPLACE_STYPEID(str, key, "")

#define $SYMBOLS_STYPEID \
{ std::string { cttypeid<std::string>.name() }, "std::string" }, \
{ std::string { cttypeid<std::string_view>.name() }, "std::string_view" }, \
{ std::string { cttypeid<std::size_t>.name() }, "std::size_t" },

#define $GET_SYMBOL_LIST_STYPEID() static std::pair<std::string, std::string> SYMBOL_LIST[] { $SYMBOLS_STYPEID }

    namespace {
        void
        replace_all_instances(std::string& base, const std::string& key, const std::string& value) {
            if(not key.empty()) {
                auto location = base.find(key);
                while(location != std::string::npos) {
                    base.replace(location, key.size(), value);
                    location = base.find(key);
                }
            }
        }
    }

    struct stypeid final : demangler {
        std::optional<std::string> demangled;
        template <typename T> stypeid(T &&t) { this->demangled = demangler::demangle(typeid(t).name()); };
        std::string name() { return this->demangled.value_or("(null)"); }
        std::string name() const { return this->demangled.value_or("(null)"); }
        friend std::ostream& operator << (std::ostream& os, const stypeid id) { return os << id.name(); }

        static void
        replace_templates(std::string& str) {

        }

        static std::string
        demangle_symbol(std::string mangled) {
            std::string extra {};
            if($REMOVE_STYPEID(mangled, "_GLOBAL__sub_I_")) extra += "static ";

            auto out = demangler::demangle(mangled).value_or(mangled);
            replace_templates(mangled);
            $GET_SYMBOL_LIST_STYPEID();
            for(auto&& [k, v] : SYMBOL_LIST) {
                replace_all_instances(out, k, v);
            }

            return extra + out;
        }

        static std::string
        demangle_symbol(std::string_view mangled) {
            return demangle_symbol(std::string { mangled });
        }

        template <typename T>
        static std::string
        demangle_symbol(detail::cttypeid_impl<T> mangled) {
            return demangle_symbol(std::string { mangled.name() });
        }
    };
}

using zen::stypeid;

#endif //ENGINE_STYPEID_HPP
