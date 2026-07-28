#pragma once
#include <array>
#include <string_view>


template<typename T>
struct dtoFields {
    static_assert(sizeof(T) == 0,
        "dtoFields<T> not specialized for this DTO type");
};
