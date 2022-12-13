#ifndef ENGINE_JSONIFY_HPP
#define ENGINE_JSONIFY_HPP

#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include <loophole/type-loophole.h>
#include <nlohmann_json.hpp>

#include <zen/util/custom_exception.hpp>
#include "cttypeid.hpp"

#ifdef ZEN_JSONIFY_NO_ASSERT
#  define $JSONIFY_ASSERT(eval, msg) throw custom_assert { msg }
#else
#  define $JSONIFY_ASSERT(eval, msg) static_assert(eval, "error: " msg)
#endif

namespace zen::meta {
    template <typename...TT>
    using tl_luple = luple_t<luple_ns::type_list<TT...>>;

    namespace detail {
        namespace {
            template <typename T>
            std::string
            add_quotes(T&& t) {
                return '"' + std::string { t } + '"';
            }
        }

        template <typename U, typename...TT>
        std::string
        serialize_struct(tl_luple<TT...>&);

        template <typename T>
        concept exists_json_overload = requires(nlohmann::json& j, T t) { nlohmann::to_json(j, t); };

        template <typename T>
        concept exists_string_overload = requires(T t) { std::to_string(t); };

        template <typename T>
        auto&
        get_struct_tuple(T& t) {
            using U = std::decay_t<T>;

            if constexpr(std::is_aggregate_v<U>) {
                using data_tlist = loophole_ns::as_type_list<U>;
                using data_luple = luple_t<data_tlist>;
                return reinterpret_cast<data_luple&>(t);
            }

            $JSONIFY_ASSERT(std::is_aggregate_v<U>, "structure must be an aggregate type.");
        }

        template <typename T>
        std::string
        serialize_member_helper(T&& t) {
            using U = std::decay_t<T>;

            if constexpr(std::is_null_pointer_v<U>) {
                return { '{' + add_quotes( cttypeid<U>.name() ) + ":0}" };
            }
            else if constexpr(std::is_pointer_v<U>) {
                if constexpr(std::is_same_v<U, void*>) {
                    return { '{' + add_quotes( cttypeid<U>.name() ) + ":0}" };
                }
                else {
                    std::string str { '{' + add_quotes( cttypeid<U>.name() ) + ':' };
                    if(t) str += serialize_member_helper(*t);
                    else str.push_back('0');
                    return { str + '}' };
                }
            }
            else if constexpr(exists_string_overload<U>) {
                return { '{' + add_quotes( cttypeid<U>.name() ) + ':' + std::to_string(t) + '}' };
            }
            else if constexpr(exists_json_overload<T>) {
                nlohmann::json j;
                nlohmann::to_json(j, t);
                return { '{' + add_quotes( cttypeid<U>.name() ) + ':' + j.dump() + '}' };
            }
            else if constexpr(std::is_aggregate_v<U>) {
                auto& l = get_struct_tuple(t);
                return serialize_struct<U>(l);
            }

            throw custom_exception {
                    "could not find serialization overload for " + add_quotes( cttypeid<U>.name() )
            };
        }

        template <typename T>
        nlohmann::json
        serialize_member(T&& t) {
            auto str = serialize_member_helper(t);
            return nlohmann::json::parse(str.begin(), str.end());
        }


        template <typename U, typename...TT>
        std::string
        serialize_struct(tl_luple<TT...>& l) {
            if constexpr(std::is_aggregate_v<U>) {
                auto deconstruct = [&] <std::size_t...NN> (std::index_sequence<NN...>) {
                    return std::vector<std::string> {
                            serialize_member_helper(get<NN>(l))...
                    };
                };

                auto vec { deconstruct(std::make_index_sequence<sizeof...(TT)>()) };
                std::span<std::string> s ( vec.begin(), vec.size() - 1 );
                std::string out_str { '{' + add_quotes( cttypeid<U>.name() ) + ":[" };

                for(auto& str : s) out_str += str + ',';
                out_str += vec.back();
                return { out_str + "]}" };
            }

            $JSONIFY_ASSERT(std::is_aggregate_v<U>, "structure must be an aggregate type.");
        }
    }

    template <typename T>
    concept serializable = requires(T t) {
        detail::serialize_member(t);
    };

    template <typename T>
    nlohmann::json
    serialize(T&& t) {
        using U = std::decay_t<T>;
        if constexpr(std::is_aggregate_v<U>) {
            auto& l = detail::get_struct_tuple(t);

            auto data { detail::serialize_struct<U>(l) };
            return nlohmann::json::parse(data.begin(), data.end());
        }
        else if constexpr(serializable<T>) {
            return detail::serialize_member(t);
        }

        $JSONIFY_ASSERT(serializable<T>, "object is not serializable.");
    }
}

#endif //ENGINE_JSONIFY_HPP
