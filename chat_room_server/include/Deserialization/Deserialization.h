#pragma once

#include <string>
#include <stdexcept>
#include <type_traits>
#include "./DTO/RequestStruct.h"
#include "./DTO/DtoFieldsValues.h"

template<typename, typename = void>
struct has_optional : std::false_type {};

template<typename T>
struct has_optional<T, std::void_t<decltype(dtoFields<T>::optional)>>
    : std::true_type {};

template<typename, typename = void>
struct has_strings : std::false_type {};

template<typename T>
struct has_strings<T, std::void_t<decltype(dtoFields<T>::strings)>>
    : std::true_type {};

template<typename, typename = void>
struct has_ints : std::false_type {};

template<typename T>
struct has_ints<T, std::void_t<decltype(dtoFields<T>::ints)>>
    : std::true_type {};

template<typename, typename = void>
struct has_bools : std::false_type {};

template<typename T>
struct has_bools<T, std::void_t<decltype(dtoFields<T>::bools)>>
    : std::true_type {};


template<typename T>
T deserialize(const RequestStruct& req)
{
    T dto{};
    const auto& j = req.data;

    if constexpr (has_strings<T>::value) {
        for (auto [key, member] : dtoFields<T>::strings) {
            std::string k(key);
            if (j.contains(k) && !j[k].is_null())
                dto.*member = j[k].is_string() ? j[k].get<std::string>()
                                                : j[k].dump();
            else
                dto.*member = std::string{};
        }
    }

    if constexpr (has_ints<T>::value) {
        for (auto [key, member] : dtoFields<T>::ints) {
            std::string k(key);
            if (!j.contains(k) || j[k].is_null())
                continue;
            const auto& val = j[k];
            if (val.is_number_integer() || val.is_number_unsigned())
                dto.*member = val.get<int>();
            else if (val.is_number_float())
                dto.*member = static_cast<int>(val.get<double>());
            else if (val.is_string())
                dto.*member = std::stoi(val.get<std::string>());
            // else: leave default 0 — handler may set from JWT (authenticatedUserId)
        }
    }

    if constexpr (has_optional<T>::value) {
        for (auto [key, member] : dtoFields<T>::optional) {
            std::string k(key);
            if (!j.contains(k) || j[k].is_null()) {
                dto.*member = std::nullopt;
                continue;
            }
            const auto& val = j[k];
            int v;
            if (val.is_number_integer() || val.is_number_unsigned())
                v = val.get<int>();
            else if (val.is_number_float())
                v = static_cast<int>(val.get<double>());
            else if (val.is_string())
                v = std::stoi(val.get<std::string>());
            else
                v = 0;

            if (v)
                dto.*member = v;
            else
                dto.*member = std::nullopt;
        }
    }

    if constexpr (has_bools<T>::value) {
        for (auto [key, member] : dtoFields<T>::bools) {
            std::string k(key);
            if (!j.contains(k) || j[k].is_null())
                continue;
            const auto& val = j[k];
            if (val.is_boolean())
                dto.*member = val.get<bool>();
            else if (val.is_string())
                dto.*member = val.get<std::string>() == "true";
        }
    }

    dto.sessionId = req.sessionID;

    return dto;
}
