#pragma once
#include <cstdint>

namespace core {
    enum class GraphicsFormat : int32_t {
        unknown = 0,

        // float

        r32_g32_b32_a32_float,
        r32_g32_b32_float,
        r32_g32_float,
        r32_float,

        r16_g16_b16_a16_float,
        r16_g16_float,
        r16_float,

        // unsigned float

        r11_g11_b10_ufloat,

        // int

        r32_g32_b32_a32_int,
        r32_g32_b32_int,
        r32_g32_int,
        r32_int,

        r16_g16_b16_a16_int,
        r16_g16_int,
        r16_int,

        r8_g8_b8_a8_int,
        r8_g8_int,
        r8_int,

        // uint

        r32_g32_b32_a32_uint,
        r32_g32_b32_uint,
        r32_g32_uint,
        r32_uint,

        r16_g16_b16_a16_uint,
        r16_g16_uint,
        r16_uint,

        r10_g10_b10_a2_uint,

        r8_g8_b8_a8_uint,
        r8_g8_uint,
        r8_uint,

        // signed normalized

        r16_g16_b16_a16_snorm,
        r16_g16_snorm,
        r16_snorm,

        r8_g8_b8_a8_snorm,
        r8_g8_snorm,
        r8_snorm,

        // unsigned normalized

        r16_g16_b16_a16_unorm,
        r16_g16_unorm,
        r16_unorm,

        r10_g10_b10_a2_unorm,

        r8_g8_b8_a8_unorm,
        r8_g8_unorm,
        r8_unorm,
        a8_unorm,

        b8_g8_r8_a8_unorm,

        b5_g6_r5_unorm,
        b5_g5_r5_a1_unorm,

        // sRGB

        r8_g8_b8_a8_unorm_srgb,
        b8_g8_r8_a8_unorm_srgb,

        // depth stencil

        d32_float,
        d32_float_s8_x24_uint,

        d24_unorm_s8_uint,

        d16_unorm,

        // bc

        bc1_unorm,
        bc1_unorm_srgb,

        bc2_unorm,
        bc2_unorm_srgb,

        bc3_unorm,
        bc3_unorm_srgb,

        bc4_unorm,
        bc4_snorm,

        bc5_unorm,
        bc5_snorm,

        bc6h_uf16,
        bc6h_sf16,

        bc7_unorm,
        bc7_unorm_srgb,

        // other

        r32_g32_b32_a32_typeless,
        r32_g32_b32_typeless,
        r32_g32_typeless,
        r32_g8_x24_typeless,
        r32_float_x8_x24_typeless,
        x32_typeless_g8_x24_uint,
        r32_typeless,

        r24_g8_typeless,
        r24_unorm_x8_typeless,
        x24_typeless_g8_uint,

        r16_g16_b16_a16_typeless,
        r16_g16_typeless,
        r16_typeless,

        r10_g10_b10_a2_typeless,
        r10_g10_b10_xr_bias_a2_unorm,

        r8_g8_b8_a8_typeless,
        r8_g8_typeless,
        r8_typeless,

        b8_g8_r8_a8_typeless,
        b8_g8_r8_x8_typeless,

        bc1_typeless,
        bc2_typeless,
        bc3_typeless,
        bc4_typeless,
        bc5_typeless,
        bc6h_typeless,
        bc7_typeless,
    };
}
