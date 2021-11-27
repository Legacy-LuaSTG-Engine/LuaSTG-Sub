#pragma once

// basic type primitives
#include "slow/NativeType.hpp"

// basic types
namespace slow
{
    struct f32x2
    {
        union
        {
            struct
            {
                f32 x;
                f32 y;
            };
            f32 data[2];
        };
        f32x2() : x(0.0f), y(0.0f) {}
        f32x2(f32 x_, f32 y_) : x(x_), y(y_) {}
    };
    struct f32x3
    {
        union
        {
            struct
            {
                f32 x;
                f32 y;
                f32 z;
            };
            struct
            {
                f32 r;
                f32 g;
                f32 b;
            };
            f32 data[3];
        };
        f32x3() : x(0.0f), y(0.0f), z(0.0f) {}
        f32x3(f32 x_, f32 y_, f32 z_) : x(x_), y(y_), z(z_) {}
    };
    struct f32x4
    {
        union
        {
            struct
            {
                f32 x;
                f32 y;
                f32 z;
                f32 w;
            };
            struct
            {
                f32 r;
                f32 g;
                f32 b;
                f32 a;
            };
            struct
            {
                f32 left;
                f32 top;
                f32 right;
                f32 bottom;
            };
            f32 data[4];
        };
        f32x4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        f32x4(f32 x_, f32 y_, f32 z_, f32 w_) : x(x_), y(y_), z(z_), w(w_) {}
    };
}
