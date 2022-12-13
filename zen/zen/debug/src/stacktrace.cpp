#include <zen/debug/stacktrace.hpp>

#include <cstring>
#include <utility>

#include <zen/os/architecture.hpp>
#include <zen/os/ansi_color.hpp>
#include <zen/os/console.hpp>

#include <zen/filesystem.hpp>
#include <zen/meta/stypeid.hpp>

namespace zen::debug {
    void
    print_stacktrace_impl(const std::vector<stackframe>& stack) {
        if(not stack.empty()) {
            std::cout << "-= stacktrace =-\n";
            for(auto& frame : stack) {
                std::cout << " ---> " << reinterpret_cast<void*>(frame.function_address) << ": ";
                std::cout << ansi::yellow << stypeid::demangle_symbol(frame.function_name) << ansi::reset;
                std::cout << " in " << $PATH_SNIP_STACKTRACE(frame.module_name);
                if(not frame.file.empty()) {
                    std::cout << " (" << $PATH_SNIP_STACKTRACE(frame.file) << ":" << frame.line << ")\n";
                }
                else std::cout << '\n';
            }
        }
        else {
            std::cout << ansi::red;
            std::cout << "<stacktrace failed>\n" << ansi::reset;
        }
        std::cout << std::endl;
    }

    void
    print_stacktrace(const std::exception_ptr& what) {
        zen::os::global_console::construct();
        zen::os::enable_ansi_color();
        zen::os::set_console_mode(ENABLE_WRAP_AT_EOL_OUTPUT);
        zen::os::set_cursor_mode({ .bVisible = false });

        try {
            if(what) std::rethrow_exception(what);
        }
        catch(const stacktrace_exception& st_ex) {
            auto info{ st_ex.info() };
            const auto& stack = stack_representation::get();
            std::cout << ansi::yellow << "uh oh" << ansi::reset << "! a "
                      << ansi::red << "fatal" << ansi::reset
                      << " error occured...\n";
            std::cout << ansi::yellow << "who" << ansi::reset << ": " << info.function << "\n";
            std::cout << ansi::yellow << "what" << ansi::reset << ": " << st_ex.what() << "\n";
            std::cout << ansi::yellow << "where" << ansi::reset << ": " << info.file << ':' << info.line << "\n\n";

            print_stacktrace_impl(stack);
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << "\n\n";
        }
    }

    std::vector<stackframe>
    get_stack_frames() {
        auto machine = os::get_current_architecture();
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

        CONTEXT context {};
        std::memset(&context, 0, sizeof(context));
        context.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&context);

        symbol_handler sym { process };
        SymSetOptions(SYMOPT_LOAD_LINES);

        STACKFRAME current_frame { get_current_stack_frame(context, AddrModeFlat) };
        std::vector<stackframe> frame_registry {};

        DWORD machine_type;
        using os::arches;
        switch(machine) {
            case arches::amd64:
                machine_type = IMAGE_FILE_MACHINE_AMD64;
                break;
            case arches::itanium64:
                machine_type = IMAGE_FILE_MACHINE_IA64;
                break;
            case arches::x86:
                machine_type = IMAGE_FILE_MACHINE_I386;
                break;
            case arches::arm: [[fallthrough]];
            case arches::arm64: [[fallthrough]];
            case arches::unknown:
                return frame_registry;
        }

        auto stack_walk = [&]() {
            return StackWalk(machine_type, process, thread,
                             &current_frame, &context, NULL,
                             SymFunctionTableAccess64,
                             SymGetModuleBase64, NULL);
        };

        stack_walk();
        while(stack_walk()) {
            DWORD64 function_address;
            std::string module_name;
            std::string function_name;
            std::string file;
            unsigned int line = 0;

            function_address = current_frame.AddrPC.Offset;

            DWORD module_base = $invoke_winapi(SymGetModuleBase, 0)(process, current_frame.AddrPC.Offset);
            char module_buff[MAX_PATH] {};
            if (module_base && GetModuleFileNameA(NULL, module_buff , MAX_PATH)) {
                module_name = module_buff;
            }
            else module_name = "unknown module";

            char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255] {};
            auto symbol = (PIMAGEHLP_SYMBOL)symbol_buffer;
            symbol->SizeOfStruct = (sizeof(IMAGEHLP_SYMBOL)) + 255;
            symbol->MaxNameLength = 254;

            if (SymGetSymFromAddr(process, current_frame.AddrPC.Offset, NULL, symbol)) {
                function_name = symbol->Name;
            }
            else function_name = "unknown function";

            DWORD  offset = 0;
            IMAGEHLP_LINE iline {
                    .SizeOfStruct = sizeof(IMAGEHLP_LINE),
            };


            if (SymGetLineFromAddr(process, current_frame.AddrPC.Offset, &offset, &iline)) {
                file = iline.FileName;
                line = iline.LineNumber;
                frame_registry.emplace_back(function_address, module_name, function_name, file, line);
            }
            else frame_registry.emplace_back(function_address, module_name, function_name);
        }


        return frame_registry;
    }

    STACKFRAME
    get_current_stack_frame(const CONTEXT& ctx, ADDRESS_MODE mode) noexcept {
#if _WIN64
        return {
                .AddrPC {
                        .Offset = ctx.Rip,
                        .Mode = mode,
                },
                .AddrFrame {
                        .Offset = ctx.Rbp,
                        .Mode = mode,
                },
                .AddrStack {
                        .Offset = ctx.Rsp,
                        .Mode = mode,
                },
        };
#else
        return {
                .AddrPC {
                        .Offset = ctx.Eip,
                        .Mode = mode,
                },
                .AddrFrame {
                        .Offset = ctx.Ebp,
                        .Mode = mode,
                },
                .AddrStack {
                        .Offset = ctx.Esp,
                        .Mode = mode,
                },
        };
#endif
    }


    symbol_handler::symbol_handler() : process(GetCurrentProcess()) {
        ret = $invoke_winapi(SymInitialize, false)(process, nullptr, true);
    }

    symbol_handler::symbol_handler(HANDLE h) : process(h) {
        ret = $invoke_winapi(SymInitialize, false)(process, nullptr, true);
    }

    symbol_handler::~symbol_handler() {
        try {
            if(process) {
                $invoke_winapi(SymCleanup, false)(process);
                process = nullptr;
            }
        }
        catch(std::exception& e) {
            std::cout << e.what() << "\n\n";
            std::exit(-1);
        }
    }


    void
    stack_representation::set() {
        auto& p = get_internal();
        if(not p) {
            p = new current_stack { get_stack_frames() };
        }
        else {
            delete p;
            p = new current_stack { get_stack_frames() };
        }
        p->erase(p->begin());
    }

    void
    stack_representation::reset() {
        auto& p = get_internal();
        if(p) {
            delete p;
            p = nullptr;
        }
    }

    const stack_representation::current_stack&
    stack_representation::get() {
        auto& p = get_internal();
        if(p) return *p;
        else {
            set();
            return *p;
        }
    }

    stack_representation::current_stack*&
    stack_representation::get_internal() {
        static thread_local lifetime<current_stack> p {};
        return p.get();
    }
}