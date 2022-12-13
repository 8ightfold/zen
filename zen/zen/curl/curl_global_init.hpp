#ifndef ENGINE_CURL_GLOBAL_INIT_HPP
#define ENGINE_CURL_GLOBAL_INIT_HPP

#include <curl/curl.h>

namespace zen::curl {
    struct curl_global_wrapper {
        curl_global_wrapper() {
            curl_global_init(CURL_GLOBAL_ALL);
        }

        ~curl_global_wrapper() {
            curl_global_cleanup();
        }
    };

    const curl_global_wrapper&
    get_global_wrapper();

    namespace {
        const auto& curl_global = get_global_wrapper();
    }
}

#endif //ENGINE_CURL_GLOBAL_INIT_HPP
