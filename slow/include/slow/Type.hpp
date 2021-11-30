#pragma once

#ifndef SLOW_DLL_API
#define SLOW_DLL_API
#endif

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

    using b8 = bool;
    using b32 = u32;

    static_assert(sizeof(b8) == 1, "invalid b8 type size");
    static_assert(sizeof(b32) == 4, "invalid b32 type size");
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
    using b8 = bool;
    using b32 = u32;
}
#endif

// string
namespace slow
{
    using c8str = c8*;
    using c16str = c16*;
    using c32str = c32*;
    using c8cstr = c8 const*;
    using c16cstr = c16 const*;
    using c32cstr = c32 const*;
}

// basic types
namespace slow
{
    struct u32x4
    {
        union
        {
            struct
            {
                u32 x;
                u32 y;
                u32 z;
                u32 w;
            };
            struct
            {
                u32 left;
                u32 top;
                u32 right;
                u32 bottom;
            };
            u32 data[4];
        };
        u32x4() : x(0), y(0), z(0), w(0) {}
        u32x4(u32 x_, u32 y_, u32 z_, u32 w_) : x(x_), y(y_), z(z_), w(w_) {}
    };
    
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

    struct f32box
    {
        union
        {
            struct
            {
                f32x3 a;
                f32x3 b;
            };
            struct
            {
                f32 left;
                f32 top;
                f32 front;
                f32 right;
                f32 bottom;
                f32 back;
            };
            f32 data[6];
        };
        f32box() : a(f32x3()), b(f32x3()) {}
        f32box(f32x3 a_, f32x3 b_) : a(a_), b(b_) {}
        f32box(f32 left_, f32 top_, f32 front_, f32 right_, f32 bottom_, f32 back_)
            : left(left_), top(top_), front(front_)
            , right(right_), bottom(bottom_), back(back_) {}
    };

    struct u8view
    {
        u8 const* data;
        usize size;
        u8view() : data(nullptr), size(0) {}
        u8view(u8 const* data_, usize size_) : data(data_), size(size_) {}
        u8view(void const* data_, usize size_) : data((u8 const*)data_), size(size_) {}
    };

    struct c8view
    {
        c8 const* data;
        usize size;
        c8view() : data(""), size(0) {}
        c8view(c8 const* data_, usize size_) : data(data_), size(size_) {}
        // waring: unsafe, only avaliable for C-style null-terminal string
        c8view(c8 const* data_) : data(data_), size(0) { for (size = 0; data[size]; size += 1); }
    };

    constexpr int padding = 0;
}
