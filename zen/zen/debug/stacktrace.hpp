#ifndef ENGINE_STACKTRACE_HPP
#define ENGINE_STACKTRACE_HPP

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <windows.h>
#include <dbghelp.h>

#include <zen/meta/preprocessor.hpp>

#ifndef STACKTRACE_FULL_PATH
#  define $PATH_SNIP_STACKTRACE(fpath) \
    fs::path{ fs::path{ fpath }.parent_path().filename() / fs::path{ fpath }.filename() }.string()
#else
#  define $PATH_SNIP_STACKTRACE(fpath) std::string { fpath }
#endif


namespace zen::debug {
    struct stackframe {
        DWORD64 function_address;
        std::string module_name;
        std::string function_name;
        std::string file;
        unsigned line;

    public:
        stackframe() = default;

        stackframe(DWORD64 addr, std::string mod, std::string fn)
                : function_address(addr), module_name(std::move(mod)),
                  function_name(std::move(fn)), file(""), line(0) {}

        stackframe(DWORD64 addr, std::string mod, std::string fn, std::string file, unsigned line)
                : function_address(addr), module_name(std::move(mod)),
                  function_name(std::move(fn)), file(std::move(file)), line(line) {}
    };

    struct stacktrace_exception : std::exception {
        struct location_info {
            const char* function;
            const char* file;
            unsigned line;
        };

    public:
        explicit stacktrace_exception(const std::string&& str, const char* func, const char* file, unsigned line)
                : data(str), info_{ func, file, line } {}

        [[nodiscard]]
        const char*
        what() const noexcept override {
            std::cout << std::flush;
            return data.c_str();
        }

        [[nodiscard]]
        location_info
        info() const noexcept {
            return info_;
        }

    private:
        const std::string data;
        location_info info_;
    };

    struct symbol_handler {
        symbol_handler();

        explicit
        symbol_handler(HANDLE h);

        ~symbol_handler();

    private:
        HANDLE process = nullptr;
        BOOL ret = TRUE;
    };

    struct stack_representation {
        using current_stack = std::vector<stackframe>;

        static void
        set();

        static void
        reset();

        static const stack_representation::current_stack&
        get();

    private:
        static current_stack*&
        get_internal();
    };


    void
    print_stacktrace(const std::exception_ptr&);

    void
    print_stacktrace_impl(const std::vector<stackframe>&);

    std::vector<stackframe>
    get_stack_frames();

    STACKFRAME
    get_current_stack_frame(const CONTEXT& ctx, ADDRESS_MODE mode) noexcept;
}

#define $trace_throw(...)                                                                                           \
    do {                                                                                                            \
        zen::debug::stack_representation::set();                                                                    \
        throw zen::debug::stacktrace_exception {                                                                    \
            $expand_args(__VA_ARGS__ ""), __func__, __FILE__, __LINE__                                              \
        };                                                                                                          \
    } while(0)

#define $trace_stack() zen::debug::print_stacktrace(std::current_exception())

#endif //ENGINE_STACKTRACE_HPP
