#ifndef ENGINE_V_WRAPPER_HPP
#define ENGINE_V_WRAPPER_HPP

#include <optional>
#include <string>

#include <zen/meta/cttypeid.hpp>
#include <zen/meta/stypeid.hpp>
#include <zen/util/custom_exception.hpp>

namespace zen::util {
    struct v_base {
        virtual ~v_base() = default;
        template <typename T> std::optional<T> dispatch();
        template <typename T> T& force_dispatch();

        virtual std::string
        type_name() {
            return "v_base";
        }
    };

    template <typename T>
    struct v_wrapper : public v_base {
        using type = T;
        T data;
        explicit v_wrapper(T& t) : data(t) {}
        explicit v_wrapper(T&& t) : data(std::move(t)) {}

        std::string
        type_name() override {
            std::string str = stypeid(*this).name();
            if(auto location = str.find("zen::util::v_derived<"); location != std::string::npos) {
                str.replace(location, sizeof("zen::util::v_derived<") - 1, "");
                str.pop_back();
            }
            return str;
        }
    };

    template <typename T>
    std::optional<T> v_base::dispatch() {
        if(auto* d = dynamic_cast<v_wrapper<T>*>(this); d != nullptr) {
            return {d->data};
        }
        else return {};
    }

    template <typename T>
    T& v_base::force_dispatch() {
        if(auto* d = dynamic_cast<v_wrapper<T>*>(this); d != nullptr) {
            return d->data;
        }
        else throw custom_exception {
            "could not dispatch as " + cttypeid<T>.string() + " (correct type is " + type_name() + ") "
        };
    }
}

namespace zen {
    using util::v_base;
    using util::v_wrapper;
}

#endif //ENGINE_V_WRAPPER_HPP
