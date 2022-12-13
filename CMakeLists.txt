cmake_minimum_required(VERSION 3.23)
project(engine)

set(CMAKE_CXX_STANDARD 20)

set(ZEN_SOURCES
        zen/zen/curl/curl_global_init.cpp zen/zen/curl/generate_pdb.cpp zen/zen/curl/retrieve_data.cpp
        zen/zen/util/custom_exception.cpp zen/zen/util/strmanip.cpp
        zen/zen/meta/stypeid.cpp
        zen/zen/os/console.cpp
        zen/zen/debug/print_caller.cpp zen/zen/debug/stacktrace.cpp)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} C:/Program Files/JetBrains/CLion 2022.2.4/bin/cmake/win/share/cmake-3.23/Modules")
add_subdirectory(external)

add_executable(${PROJECT_NAME} main.cpp ${ZEN_SOURCES})
add_compile_options(${PROJECT_NAME} -g)
target_include_directories(${PROJECT_NAME} PUBLIC zen)
target_link_libraries(${PROJECT_NAME} PUBLIC external)
target_link_libraries(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/external/DbgHelp.lib
        ${CMAKE_SOURCE_DIR}/external/Version.lib)

add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_BINARY_DIR}/extra/bin.exe"
)