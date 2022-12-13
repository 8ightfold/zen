#ifndef ENGINE_ARCHITECTURE_HPP
#define ENGINE_ARCHITECTURE_HPP

#include <windows.h>
#include <zen/util/custom_exception.hpp>

namespace zen::os {
    enum class arches : DWORD {
        amd64 = 9,
        arm = 5,
        arm64 = 12,
        itanium64 = 6,
        x86 = 0,
        unknown = 0xffff,
    };

    arches
    get_current_architecture() noexcept {
        SYSTEM_INFO sysinfo {};
        GetNativeSystemInfo(&sysinfo);
        auto info = sysinfo.wProcessorArchitecture;
        arches arch { static_cast<arches>(info) };
        return arch;
    }

    void
    get_thread_context(HANDLE thread, CONTEXT& ctx) {
        try {
            $invoke_winapi(GetThreadContext, false)(thread, &ctx);
        }
        catch(std::exception& e) {
            std::cerr << e.what() << "\n\n";
            std::exit(-1);
        }
    }
}

#endif //ENGINE_ARCHITECTURE_HPP
