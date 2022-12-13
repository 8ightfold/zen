#ifndef ENGINE_OBJECT_REGISTRY_HPP
#define ENGINE_OBJECT_REGISTRY_HPP

#include <fstream>
#include <typeinfo>

#include <zen/meta/stypeid.hpp>
#include <zen/meta/jsonify.hpp>
#include <zen/util/v_wrapper.hpp>

namespace zen {
    using callback_t = v_base*(*)(v_base*, const std::vector<std::string>& args);

    template <typename T>
    struct serialize_callback {
        static v_base*
        callback(v_base* v, const std::vector<std::string>& vec) {
            std::string name = vec.at(0);
            std::optional<T> opt = v->dispatch<T>();
            if(opt.has_value()) {
                T& t = opt.value();
                nlohmann::json j;
                j[name] = meta::serialize(t);
                return new v_wrapper { j };
            }
            throw custom_exception {
                    "incorrect type " + cttypeid<T>.string() + " given to serializer (correct type is " + v->type_name() + " )"
            };
        }
    };


    struct object_registry {
    private:
        struct reserved_callbacks {
            callback_t serialize;
        };

        struct registry_entry {
            v_base* entry = nullptr;
            reserved_callbacks reserved {};
            std::map<std::string, callback_t> callbacks {};
        };

    public:
        using registry_t = std::map<std::string, registry_entry>;

        static void
        construct() {
            auto& internal = internal_get();
            if(not internal) internal = new object_registry {};
        }

        static void
        destruct() {
            auto& internal = internal_get();
            if(internal) {
                delete internal;
                internal = nullptr;
            }
        }


        template <typename T>
        static void
        register_value(const std::string& name, T&& t) {
            using U = std::decay_t<T>;
            auto& internal = internal_get();
            if(internal) {
                registry_t& reg = internal->data;

                if(not reg.contains(name)) {
                    callback_t call = serialize_callback<U>::callback;
                    reg[name] = registry_entry { new v_wrapper { t } };
                    reg[name].reserved.serialize = call;
                }
                else {}
            }
        }


        template <typename T>
        static T&
        get_handle(const std::string& name) {
            using U = std::decay_t<T>;
            auto& internal = internal_get();
            if(internal) {
                registry_t& reg = internal->data;

                if(reg.contains(name)) {
                    auto* entry = reg[name].entry;
                    return entry->force_dispatch<T>();
                }
                else throw custom_exception {
                    "could not locate entry \"" + name + '"'
                };
            }
            throw custom_exception {
                "registry has not been instantiated."
            };
        }


        static nlohmann::json
        serialize_entry(const std::string& name) {
            auto& internal = internal_get();
            if(internal) {
                registry_t& reg = internal->data;
                if(reg.contains(name)) {
                    callback_t call = reg[name].reserved.serialize;
                    auto* json = call(reg[name].entry, { name });
                    return json->force_dispatch<nlohmann::json>();
                }
            }
            return {};
        }

        static void
        serialize_all_to(fs::path filepath) {
            auto& internal = internal_get();
            if(internal) {
                registry_t& reg = internal->data;
                nlohmann::json json_output;
                for(auto&& [name, data] : reg) {
                    callback_t call = data.reserved.serialize;
                    auto* json = call(data.entry, { name });
                    json_output += json->force_dispatch<nlohmann::json>();
                }

                std::string str_out { json_output.dump() };
                std::ofstream of { filepath, std::ios::binary };
                if(of.is_open()) {
                    of << str_out;
                    of << std::flush;
                    of.close();
                }
            }
        }


        ~object_registry() {
            for(auto& [key, val] : data) delete val.entry;
        }

    private:
        static object_registry*&
        internal_get() {
            static lifetime<object_registry> reg;
            return reg.get();
        }

    private:
        registry_t data;
    };
}

#endif //ENGINE_OBJECT_REGISTRY_HPP
