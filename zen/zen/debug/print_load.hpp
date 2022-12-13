#ifndef ENGINE_PRINT_LOAD_HPP
#define ENGINE_PRINT_LOAD_HPP

#include <iostream>
#include <libloaderapi.h>
#include <zen/os/ansi_color.hpp>

namespace zen::debug {
    static bool
    is_loaded(std::string str) {
        return GetModuleHandleA(str.c_str());
    }

    void print_load(const char* name) {
        std::cout << "is \"" << name << "\" loaded? ";
        if(is_loaded(name)) std::cout << zen::ansi::green << "yes";
        else std::cout << zen::ansi::red << "no";
        std::cout << zen::ansi::reset;
        std::cout << std::endl;
    }
}

#endif //ENGINE_PRINT_LOAD_HPP
