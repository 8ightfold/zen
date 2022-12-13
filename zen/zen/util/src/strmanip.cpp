#include <zen/util/strmanip.hpp>

#include <cstring>
#include <vector>
#include <zen/util/custom_exception.hpp>

namespace zen::util {
    std::string narrow(const std::wstring& wstr) {
        int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),-1,
            nullptr, 0, nullptr, nullptr);
        if(size <= 0) throw custom_exception {
            "error: narrowed size " + std::to_string(size) + " is invalid."
        };

        std::string out {};
        out.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1,
            out.data(), size, nullptr, nullptr);

        return out;
    }

    std::wstring widen(const std::string& str) {
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1,nullptr, 0);
        if(size <= 0) throw custom_exception {
            "error: widened size " + std::to_string(size) + " is invalid."
        };

        std::wstring out {};
        out.resize(size);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1,out.data(), size);

        return out;
    }
}
