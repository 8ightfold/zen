#include <zen/util/custom_exception.hpp>

#include <strsafe.h>

namespace zen::util {
    void
    throw_last_error(LPTSTR lpszFunction) {
        LPVOID lpMsgBuf;
        LPTSTR lpOutBuf;
        DWORD err_code = GetLastError();

        FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // NOLINT
                (LPTSTR) &lpMsgBuf, 0, NULL);  // NOLINT

        lpOutBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        std::size_t out_size = LocalSize(lpOutBuf) / sizeof(TCHAR);
        StringCchPrintf((LPTSTR)lpOutBuf,out_size,
                        TEXT("os error: \"%s\" failed with error %d; %s"),
                        lpszFunction, err_code, lpMsgBuf);

        if constexpr(std::is_same_v<LPTSTR, char*>) {
            std::string throw_buf { lpOutBuf };
            LocalFree(lpMsgBuf);
            LocalFree(lpOutBuf);
            throw custom_exception { throw_buf + "" };
        }
        else {
            using base_t = std::remove_pointer_t<LPTSTR>;
            auto& facet = std::use_facet<std::ctype<base_t>>(std::locale {});

            std::string throw_buf {};
            throw_buf.resize(out_size);
            std::transform(lpOutBuf, lpOutBuf + out_size, throw_buf.begin(),
                           [&](auto c) { return facet.narrow(c, '*'); });

            LocalFree(lpMsgBuf);
            LocalFree(lpOutBuf);
            throw custom_exception { throw_buf + "" };
        }
    }
}
