cmake_minimum_required(VERSION 3.23)

add_library(external INTERFACE)

set(CURL_USE_SCHANNEL ON)
add_subdirectory(curl)
target_link_libraries(external INTERFACE CURL::libcurl)

target_include_directories(external INTERFACE json)

target_include_directories(external INTERFACE zip)
target_sources(external INTERFACE zip/zip/zip.c)

target_include_directories(external INTERFACE alexpolt)