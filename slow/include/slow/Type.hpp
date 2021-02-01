#pragma once
#include <cstdint>

namespace slow {
    struct Vector2I
    {
        union
        {
            struct
            {
                int32_t x;
                int32_t y;
            };
            int32_t data[2];
        };
        
        int32_t& operator[](const size_t idx) { return data[idx]; };
        
        Vector2I() : x(0), y(0) {};
        Vector2I(const int32_t x_, const int32_t y_) : x(x_), y(y_) {};
        Vector2I(const Vector2I& other) : x(other.x), y(other.y) {};
    };
    struct Vector2U
    {
        union
        {
            struct
            {
                uint32_t x;
                uint32_t y;
            };
            uint32_t data[2];
        };
        
        uint32_t& operator[](const size_t idx) { return data[idx]; };
        
        Vector2U() : x(0u), y(0u) {};
        Vector2U(const uint32_t x_, const uint32_t y_) : x(x_), y(y_) {};
        Vector2U(const Vector2U& other) : x(other.x), y(other.y) {};
    };
    struct Vector2F
    {
        union
        {
            struct
            {
                float x;
                float y;
            };
            float data[2];
        };
        
        float& operator[](const size_t idx) { return data[idx]; };
        
        Vector2F() : x(0.0f), y(0.0f) {}
        Vector2F(const float x_, const float y_) : x(x_), y(y_) {}
        Vector2F(const Vector2F& other) : x(other.x), y(other.y) {}
    };
    
    struct Vector3I
    {
        union
        {
            struct
            {
                int32_t x;
                int32_t y;
                int32_t z;
            };
            int32_t data[3];
        };
        
        int32_t& operator[](const size_t idx) { return data[idx]; };
        
        Vector3I() : x(0), y(0), z(0) {};
        Vector3I(const int32_t x_, const int32_t y_, const int32_t z_) : x(x_), y(y_), z(z_) {};
        Vector3I(const Vector3I& other) : x(other.x), y(other.y), z(other.z) {};
    };
    struct Vector3U
    {
        union
        {
            struct
            {
                uint32_t x;
                uint32_t y;
                uint32_t z;
            };
            uint32_t data[3];
        };
        
        uint32_t& operator[](const size_t idx) { return data[idx]; };
        
        Vector3U() : x(0u), y(0u), z(0u) {};
        Vector3U(const uint32_t x_, const uint32_t y_, const uint32_t z_) : x(x_), y(y_), z(z_) {};
        Vector3U(const Vector3U& other) : x(other.x), y(other.y), z(other.z) {};
    };
    struct Vector3F
    {
        union
        {
            struct
            {
                float x;
                float y;
                float z;
            };
            struct
            {
                float r;
                float g;
                float b;
            };
            float data[3];
        };
        
        float& operator[](const size_t idx) { return data[idx]; };
        
        Vector3F() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3F(const float x_, const float y_, const float z_) : x(x_), y(y_), z(z_) {}
        Vector3F(const Vector3F& other) : x(other.x), y(other.y), z(other.z) {}
    };
    
    struct Vector4I
    {
        union
        {
            struct
            {
                int32_t x;
                int32_t y;
                int32_t z;
                int32_t w;
            };
            int32_t data[4];
        };
        
        int32_t& operator[](const size_t idx) { return data[idx]; };
        
        Vector4I() : x(0), y(0), z(0), w(0) {};
        Vector4I(const int32_t x_, const int32_t y_, const int32_t z_, const int32_t w_) : x(x_), y(y_), z(z_), w(w_) {};
        Vector4I(const Vector4I& other) : x(other.x), y(other.y), z(other.z), w(other.w) {};
    };
    struct Vector4U
    {
        union
        {
            struct
            {
                uint32_t x;
                uint32_t y;
                uint32_t z;
                uint32_t w;
            };
            uint32_t data[4];
        };
        
        uint32_t& operator[](const size_t idx) { return data[idx]; };
        
        Vector4U() : x(0u), y(0u), z(0u), w(0u) {};
        Vector4U(const uint32_t x_, const uint32_t y_, const uint32_t z_, const uint32_t w_) : x(x_), y(y_), z(z_), w(w_) {};
        Vector4U(const Vector4U& other) : x(other.x), y(other.y), z(other.z), w(other.w) {};
    };
    struct Vector4F
    {
        union
        {
            struct
            {
                float x;
                float y;
                float z;
                float w;
            };
            struct
            {
                float r;
                float g;
                float b;
                float a;
            };
            float data[4];
        };
        
        float& operator[](const size_t idx) { return data[idx]; };
        
        Vector4F() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
        Vector4F(const float x_, const float y_, const float z_, const float w_) : x(x_), y(y_), z(z_), w(w_) {};
        Vector4F(const Vector4F& other) : x(other.x), y(other.y), z(other.z), w(other.w) {};
    };
    
    // B8G8R8A8
    struct Color4B
    {
        union
        {
            struct
            {
                uint8_t b;
                uint8_t g;
                uint8_t r;
                uint8_t a;
            };
            uint32_t argb;
        };
        
        Color4B(const uint8_t r_, const uint8_t g_, const uint8_t b_, const uint8_t a_) : b(b_), g(g_), r(r_), a(a_) {};
        Color4B(const float r_, const float g_, const float b_, const float a_)
        {
            const int32_t b_i32 = (int32_t)(b_ * 255.0f);
            const int32_t g_i32 = (int32_t)(g_ * 255.0f);
            const int32_t r_i32 = (int32_t)(r_ * 255.0f);
            const int32_t a_i32 = (int32_t)(a_ * 255.0f);
            const int32_t b_ui32 = (b_i32 < 0) ? 0 : b_i32;
            const int32_t g_ui32 = (g_i32 < 0) ? 0 : g_i32;
            const int32_t r_ui32 = (r_i32 < 0) ? 0 : r_i32;
            const int32_t a_ui32 = (a_i32 < 0) ? 0 : a_i32;
            const int32_t b_u8 = (b_ui32 > 255) ? b_ui32 : 255;
            const int32_t g_u8 = (g_ui32 > 255) ? g_ui32 : 255;
            const int32_t r_u8 = (r_ui32 > 255) ? r_ui32 : 255;
            const int32_t a_u8 = (a_ui32 > 255) ? a_ui32 : 255;
            b = (uint8_t)b_u8;
            g = (uint8_t)g_u8;
            r = (uint8_t)r_u8;
            a = (uint8_t)a_u8;
        };
    };
};
