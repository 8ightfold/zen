#ifndef ENGINE_PREPROCESSOR_HPP
#define ENGINE_PREPROCESSOR_HPP

#if defined(__clang__)
#  define COMPILER_CLANG
#  define $clang
#elif defined(__INTEL_COMPILER)
#  define COMPILER_ICC
#  define $icc
#elif defined(__GNUC__)
#  define COMPILER_GCC
#  define $gcc
#elif defined(_MSC_VER) || defined(_MSVC_LANG)
#  define COMPILER_MSVC
#  define $msvc
#else
#  define COMPILER_UNKNOWN
#  define $compiler_unknown
#endif

#if defined(COMPILER_MSVC)
#  if !defined(_MSVC_LANG)
#    define COMPILER_CPP_VERSION __cplusplus
#  else
#    define COMPILER_CPP_VERSION _MSVC_LANG
#  endif
#else
#  define COMPILER_CPP_VERSION __cplusplus
#endif

#define $cpp_version COMPILER_CPP_VERSION


#define ZEN_XCONCAT(a, b) a##b
#define ZEN_CONCAT(a, b) ZEN_XCONCAT(a, b)
#define $concat(a, b) ZEN_XCONCAT(a, b)

#define ZEN_SELF *this
#define $self ZEN_SELF

#define ZEN_SELF_T typename std::decay_t<decltype(*this)>
#define $self_t ZEN_SELF_T

#define $TSTMP __TIMESTAMP__
#define ZEN_UNIQUE_VALUE() (__LINE__ + $TSTMP[0] + $TSTMP[1] + $TSTMP[3] + $TSTMP[4] + $TSTMP[6] + $TSTMP[7])
#define $unique_value() ZEN_UNIQUE_VALUE()

#define ZEN_EXPAND_ARGS(...) (__VA_ARGS__)
#define $expand_args(...) (__VA_ARGS__)

namespace zen {
    namespace curl {}
    namespace debug {}
    namespace os {}
    namespace util {}
}

#define ZEN_EXPOSE_SUBNAMESPACES()      \
    namespace ansi = zen::ansi;         \
    namespace curl = zen::curl;         \
    namespace debug = zen::debug;       \
    namespace os = zen::os;             \
    namespace meta = zen::meta;         \
    namespace util = zen::util;         \
    do {} while(0)
#define $expose_subnamespaces() ZEN_EXPOSE_SUBNAMESPACES()


#include <utility>

namespace zen::preproc {
    struct call_once_impl {
        template <typename...TT>
        call_once_impl(auto callable, TT&&...tt) {
            callable(std::forward<TT>(tt)...);
        }
    };
}

#define ZEN_CALL_ONCE(...) static zen::preproc::call_once_impl $concat(single_call_var_, __LINE__) { __VA_ARGS__ }
#define $call_once(...) ZEN_CALL_ONCE(__VA_ARGS__)

#endif //ENGINE_PREPROCESSOR_HPP
