#include <zen/meta/stypeid.hpp>

namespace zen {
#if __has_include(<cxxabi.h>)
    std::optional<std::string>
    demangler::demangle(std::string mangled) {
        int status = 0; std::string demangled;
        char* tmp = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
        switch(status) {
            case  0: demangled = std::string(tmp); free(tmp);                 return demangled;
            case -1: $stypeid_log("Memory allocation failed for %s.\n", mangled.c_str());   return {};
            case -2: $stypeid_log("%s is an invalid name under ABI.\n", mangled.c_str());   return {};
            default: $stypeid_log("Fatal error occured demangling %s.\n", mangled.c_str()); return {};
        }
    }
#else
    std::optional<std::string> demangler::demangle(std::string mangled) { return std::string{mangled}; }
#endif
}
