#ifndef ENGINE_CONSOLE_HPP
#define ENGINE_CONSOLE_HPP

#include <memory>
#include <windows.h>
#include <zen/util/custom_exception.hpp>
#include "ansi_color.hpp"

namespace zen::os {
    constexpr auto console_deleter = [](HWND* ptr) { delete ptr; FreeConsole(); };
    using console_wrapper = std::unique_ptr<HWND, decltype(console_deleter)>;

    void
    free_console();

    void
    enable_ansi_color();

    // only links when in header fsr
    console_wrapper
    alloc_console() {
        free_console();
        $invoke_winapi(AllocConsole, false)();

        auto* window = new HWND { GetConsoleWindow() };
        return console_wrapper { window };
    }


    template <typename...TT>
    void
    set_console_mode(TT...tt) {
        HANDLE console_input = $invoke_winapi(GetStdHandle, INVALID_HANDLE_VALUE)(STD_OUTPUT_HANDLE);
        $invoke_winapi(SetConsoleMode, 0)(console_input, (... | static_cast<DWORD>(tt)));
    }

    void
    set_cursor_mode(CONSOLE_CURSOR_INFO info);


    struct global_console {
        static void
        construct() {
            if(GetConsoleWindow() == NULL) {
                if(AllocConsole()) {
                    auto& wnd = get_internal();
                    delete wnd;
                    wnd = new HWND { GetConsoleWindow() };
                }
            }
        }

        static void
        reset() {
            if(GetConsoleWindow() != NULL) FreeConsole();
            if(AllocConsole()) {
                auto& wnd = get_internal();
                delete wnd;
                wnd = new HWND { GetConsoleWindow() };
            }
        }

        static void
        destruct() {
            if(GetConsoleWindow() != NULL) FreeConsole();
            auto& wnd = get_internal();
            delete wnd;
            wnd = nullptr;
        }

    private:
        struct console_deleter {
            void
            operator()(HWND*& ptr) {
                delete ptr;
                if(GetConsoleWindow() != NULL) {
                    FreeConsole();
                }
            }
        };

        using console_rep = lifetime<HWND, console_deleter>;

        static HWND*&
        get_internal() {
            static console_rep p {};
            return p.get();
        }
    };
}

#endif //ENGINE_CONSOLE_HPP
