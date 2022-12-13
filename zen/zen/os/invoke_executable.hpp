#ifndef ENGINE_INVOKE_EXECUTABLE_HPP
#define ENGINE_INVOKE_EXECUTABLE_HPP

#include <zen/filesystem.hpp>
#include <zen/util/custom_exception.hpp>
#include <windows.h>

namespace zen::os {
    std::string
    invoke_executable(fs::path exepath, const std::string& out_args) {
        std::string path;
        if(exepath.has_relative_path()) {
            path = fs::absolute(exepath).string();
        }
        else path = exepath.string();

        LPCTSTR module_name = path.c_str();

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        char* cmds = new char[32767] { 0 };
        strcpy(cmds, out_args.data());

        $invoke_winapi(CreateProcess, 0)(module_name, cmds, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

        std::string out = cmds;
        out.shrink_to_fit();
        delete[] cmds;
        return out;
    }
}

#endif //ENGINE_INVOKE_EXECUTABLE_HPP
