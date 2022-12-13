#ifndef ENGINE_RETRIEVE_DATA_HPP
#define ENGINE_RETRIEVE_DATA_HPP

#include "curl_global_init.hpp"

#include <string>
#include <vector>

namespace zen::curl::api {
    std::string
    release_request(std::string user, std::string repo);

    std::vector<char>
    download_bytes(std::string url);
}

#endif //ENGINE_RETRIEVE_DATA_HPP
