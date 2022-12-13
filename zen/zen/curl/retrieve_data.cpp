#include "retrieve_data.hpp"

#include <cstring>
#include <iostream>

namespace zen::curl::api {
    std::size_t
    write_function(void* ptr, std::size_t size, std::size_t nmemb, std::string* data) {
        data->append((char*) ptr, size * nmemb);
        return size * nmemb;
    }

    std::string
    release_request(std::string user, std::string repo) {
        CURL* curl = curl_easy_init();
        CURLcode res;
        auto url { "https://api.github.com/repos/" + user + '/' + repo + "/releases/latest" };

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

            std::string response_string;
            std::string header_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl = NULL;

            if(res != CURLE_OK) {
                std::cout << "curl_easy_perform failed in curl::api::release_request: " << curl_easy_strerror(res) << '\n';
                return "";
            }

            return response_string;
        }
        return "";
    }


    std::size_t
    alloc_mem_callback(char* contents, std::size_t size, std::size_t nmem, void* userp) {
        std::size_t realsize = size * nmem;
        auto* vec = reinterpret_cast<std::vector<char>*>(userp);
        std::size_t old_size = vec->size();

        vec->resize(old_size + realsize);
        std::memcpy(&(vec->data()[old_size]), contents, realsize);

        return realsize;
    }

    std::vector<char>
    download_bytes(std::string url) {
        CURL* curl = curl_easy_init();
        CURLcode res;
        std::vector<char> bytes {};

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, alloc_mem_callback);
            res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&bytes));

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl = NULL;

            if(res != CURLE_OK) {
                std::cout << "curl_easy_perform failed in curl::api::download_bytes: " << curl_easy_strerror(res) << '\n';
                return {};
            }

            return bytes;
        }
        return {};
    }
}
