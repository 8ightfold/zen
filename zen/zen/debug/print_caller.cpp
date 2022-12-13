#include "print_caller.hpp"
#include <zen/meta/preprocessor.hpp>
#include <exception>
#include <vector>

#ifdef $gcc
#  define $internal_front sizeof("with ")
#  define $internal_delim ';'
#elif defined($clang)
#  define $internal_front sizeof("")
#  define $internal_delim ','
#else
#  define $internal_front sizeof("")
#  define $internal_delim '\0'
#endif


namespace zen::debug {
    namespace detail {
        static void
        replace_all_instances(std::string& base, const std::string& key, const std::string& value) {
            if(not key.empty()) {
                auto location = base.find(key);
                while(location != std::string::npos) {
                    base.replace(location, key.size(), value);
                    location = base.find(key);
                }
            }
        }

        static auto
        build_typename_map(std::string_view sv) {
            std::vector<std::string> names {};
            auto view { sv.substr(1, sv.size() - 2) };
            std::size_t location = $internal_front, back;

            while(location != std::string_view::npos) {
                back = view.find(" = ", location);
                back += sizeof(" = ") - 1;
                location = view.find($internal_delim, back);
                if(location != std::string::npos) {
                    std::string str { view.substr(back, location - back) };
                    names.emplace_back(str);
                    location += sizeof("* ") - 1;
                }
                else {
                    std::string str { view.substr(back) };
                    names.emplace_back(str);
                }
            }

            return names;
        }

#ifdef $gcc
        std::string
        parse_caller(std::string_view pf, std::string_view fn) {
            if(auto location = pf.find(" [with"); location != std::string_view::npos) {
                std::string out { pf.substr(0, location) };
                auto names { build_typename_map(pf.substr(location + 1)) };

                std::string template_args { fn };
                template_args += '<';
                for(auto&& v : names) {
                    template_args += v;
                    template_args += ", ";
                }
                template_args.pop_back();
                template_args.pop_back();
                template_args += '>';

                location = out.find(fn);
                out.replace(location, fn.size(), template_args);
                return out;
            }
            return std::string { pf };
        }

#elif defined($clang)
        std::string
        parse_caller(std::string_view pf, std::string_view fn) {
            if(auto location = pf.find(" = "); location != std::string_view::npos) {
                auto front = pf.rfind(" [", location);
                auto back = pf.find("]", pf.rfind(" = "));
                std::string out { pf };
                auto names { build_typename_map(pf.substr(front, back - front + 1)) };
                out.replace(front, back - front, "");

                std::string template_args { fn };
                template_args += '<';
                for(auto&& v : names) {
                    template_args += v;
                    template_args += ", ";
                }
                template_args.pop_back();
                template_args.pop_back();
                template_args += '>';

                location = out.find(fn);
                out.replace(location, fn.size(), template_args);
                return out;
            }
            return std::string { pf };
        }

#elif defined($msvc)
        std::string
        parse_caller(std::string_view pf, std::string_view) {
            return std::string { pf };
        }

#else
        std::string
        parse_caller(std::string_view, std::string_view) {
            throw caller_parse_exception{};
        }

#endif
    }

    void
    print_caller(std::string_view pf, std::string_view fn) {
        auto out { detail::parse_caller(pf, fn) };
        std::cout << "caller: " << out << "\n";
    }
}
