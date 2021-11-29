#pragma once

// basic type primitives
#ifdef _WIN32
namespace slow
{
    // Win32/Win64 model: ILP32/LLP64
    // int, long: 32bits
    // long long: 64bits
    // pointer: 32/64bits
    
    using u8 = unsigned char;
    using u16 = unsigned short int;
    using u32 = unsigned int;
    using u64 = unsigned long long int;
    
    static_assert(sizeof(u8) == 1, "invalid u8 type size");
    static_assert(sizeof(u16) == 2, "invalid u16 type size");
    static_assert(sizeof(u32) == 4, "invalid u32 type size");
    static_assert(sizeof(u64) == 8, "invalid u64 type size");
    
    using i8 = signed char;
    using i16 = signed short int;
    using i32 = signed int;
    using i64 = signed long long int;
    
    static_assert(sizeof(i8) == 1, "invalid i8 type size");
    static_assert(sizeof(i16) == 2, "invalid i16 type size");
    static_assert(sizeof(i32) == 4, "invalid i32 type size");
    static_assert(sizeof(i64) == 8, "invalid i64 type size");
    
    using f32 = float;
    using f64 = double;
    
    static_assert(sizeof(f32) == 4, "invalid f32 type size");
    static_assert(sizeof(f64) == 8, "invalid f64 type size");
    
    using c8 = char;
    using c16 = u16;
    using c32 = u32;
    
    static_assert(sizeof(c8) == 1, "invalid c8 type size");
    static_assert(sizeof(c16) == 2, "invalid c16 type size");
    static_assert(sizeof(c32) == 4, "invalid c32 type size");
    
    #ifdef _WIN64
        using usize = u64;
        using isize = i64;
    #else
        using usize = u32;
        using isize = i32;
    #endif
    
    static_assert(sizeof(usize) == sizeof(void*), "invalid usize type size");
    static_assert(sizeof(isize) == sizeof(void*), "invalid isize type size");
}
#else
#include <cstdint>
namespace slow
{
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using f32 = float;
    using f64 = double;
    #if (__cplusplus >= 202002L)
        using c8 = char8_t;
    #else
        using c8 = char;
    #endif
    #if (__cplusplus >= 201103L)
        using c16 = char16_t;
        using c32 = char32_t;
    #else
        using c16 = u16;
        using c32 = u32;
    #endif
    using usize = size_t;
    using isize = ptrdiff_t;
}
#endif

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
