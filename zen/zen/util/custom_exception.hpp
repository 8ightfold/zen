#ifndef ENGINE_CUSTOM_EXCEPTION_HPP
#define ENGINE_CUSTOM_EXCEPTION_HPP

#include <exception>
#include <iostream>
#include <string>

#include <windows.h>

namespace zen::util {
    template <typename T>
    struct deleter {
        void
        operator()(T*& ptr) {
            delete ptr;
            ptr = nullptr;
        }
    };

    template <typename T, typename D>
    concept is_deleter = requires(T*& ptr, D deleter) {
        deleter(ptr);
    };

    template <typename T, typename Dtor = deleter<T>>
    struct basic_lifetime {
        static_assert(is_deleter<T, Dtor>, "error: deleter must have operator() overloaded.");

        basic_lifetime() = default;
        explicit basic_lifetime(T* data) : data(data) {}

        [[nodiscard]]
        T*&
        get() noexcept {
            return data;
        }

        [[nodiscard]]
        const T*&
        get() const noexcept {
            return data;
        }

        [[nodiscard]]
        std::size_t
        size() {
            return (data ? 1 : 0);
        }

        basic_lifetime(const basic_lifetime&) = delete;
        basic_lifetime(basic_lifetime&&) = delete;

        basic_lifetime& operator=(const basic_lifetime&) = delete;
        basic_lifetime& operator=(basic_lifetime&&) = delete;

        ~basic_lifetime() {
            Dtor{}(data);
        }

    private:
        T* data = nullptr;
    };

    template <typename T>
    struct deleter<T[]> {
        void
        operator()(T*& ptr, std::size_t) {
            delete[] ptr;
            ptr = nullptr;
        }
    };

    template <typename T, typename D>
    concept is_array_deleter = requires(T*& ptr, std::size_t sz, D deleter) {
        deleter(ptr, sz);
    };

    template <typename T, typename Dtor>
    struct basic_lifetime<T[], Dtor> {
        static_assert(is_array_deleter<T, Dtor>, "error: deleter must have operator() overloaded.");

        basic_lifetime() = default;
        explicit basic_lifetime(T* data, std::size_t sz) : data(data), sz(sz) {}

        [[nodiscard]]
        std::size_t
        size() {
            return sz;
        }

        basic_lifetime(const basic_lifetime&) = delete;
        basic_lifetime(basic_lifetime&&) = delete;

        basic_lifetime& operator=(const basic_lifetime&) = delete;
        basic_lifetime& operator=(basic_lifetime&&) = delete;

        ~basic_lifetime() {
            Dtor{}(data);
        }

    private:
        T* data = nullptr;
        std::size_t sz = 0;
    };

    template <typename T, typename Dtor = deleter<T>>
    using lifetime = basic_lifetime<T, Dtor>;


    struct custom_exception : std::exception {
        explicit custom_exception(const std::string&& str) : data(str) {}

        [[nodiscard]]
        const char*
        what() const noexcept override {
            std::cout << std::flush;
            return data.c_str();
        }
    private:
        const std::string data;
    };


    void
    throw_last_error(LPTSTR lpszFunction);
}

namespace zen {
    using util::lifetime;
    using util::custom_exception;
    using util::throw_last_error;
}


#define ZEN_INVOKE_WINAPI(fn, err)                                                                              \
    [] <typename...internal_TT> (internal_TT&&...internal_tt) {                                                 \
        return                                                                                                  \
        [&] <typename internal_R, typename...internal_Args>                                                     \
        (internal_R(&)(internal_Args...)) {                                                                     \
            static_assert(sizeof...(internal_TT) == sizeof...(internal_Args),                                   \
                "incorrect amount of arguments for " #fn ".");                                                  \
            static_assert((std::is_convertible_v<internal_TT, internal_Args> && ...),                           \
                "invalid arguments for " #fn ".");                                                              \
            auto tmp = fn(std::forward<internal_TT>(internal_tt)...);                                           \
            if(tmp == err) {                                                                                    \
                std::string str { #fn };                                                                        \
                zen::throw_last_error(reinterpret_cast<LPTSTR>(str.data()));                                    \
            }                                                                                                   \
            return tmp;                                                                                         \
        } (fn);                                                                                                 \
    }

#define $invoke_winapi(fn, err) ZEN_INVOKE_WINAPI(fn, err)

#endif //ENGINE_CUSTOM_EXCEPTION_HPP
