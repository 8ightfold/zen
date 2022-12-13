#include "curl_global_init.hpp"

namespace zen::curl {
    const curl_global_wrapper&
    get_global_wrapper() {
        static curl_global_wrapper wrap {};
        return wrap;
    }
}