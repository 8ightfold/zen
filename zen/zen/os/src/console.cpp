#include <zen/os/console.hpp>

#include <zen/os/ansi_color.hpp>

namespace zen::os {
    void
    free_console() {
        if(GetConsoleWindow() != NULL) {
            $invoke_winapi(FreeConsole, false)();
        }
    }

    void
    enable_ansi_color() {
        HANDLE console_input = $invoke_winapi(GetStdHandle, INVALID_HANDLE_VALUE)(STD_OUTPUT_HANDLE);
        $invoke_winapi(SetConsoleMode, 0)(console_input, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }


    void
    set_cursor_mode(CONSOLE_CURSOR_INFO info) {
        HANDLE console_input = $invoke_winapi(GetStdHandle, INVALID_HANDLE_VALUE)(STD_OUTPUT_HANDLE);
        if(info.dwSize < 1 || info.dwSize > 100) info.dwSize = 1;
        $invoke_winapi(SetConsoleCursorInfo, 0)(console_input, &info);
    }
}