#ifndef ENGINE_STRMANIP_HPP
#define ENGINE_STRMANIP_HPP

#include <string>

namespace zen::util {
    std::string narrow(const std::wstring&);
    std::wstring widen(const std::string&);
}

namespace zen {
    using util::narrow;
    using util::widen;
}

#endif //ENGINE_STRMANIP_HPP
