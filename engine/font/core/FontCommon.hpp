#pragma once
#include <cstdint>

namespace core {
    enum class FontWeight : int32_t {
        thin = 100,
        extra_light = 200,
        light = 300,
        normal = 400,
        medium = 500,
        semi_bold = 600,
        bold = 700,
        extra_bold = 800,
        black = 900,
    };

    enum class FontStyle : int32_t {
        normal = 0,
        oblique = 1,
        italic = 2,
    };

    enum class FontWidth : int32_t {
        ultra_condensed = 1,
        extra_condensed = 2,
        condensed = 3,
        semi_condensed = 4,
        normal = 5,
        semi_expanded = 6,
        expanded = 7,
        extra_expanded = 8,
        ultra_expanded = 9,
    };
}
