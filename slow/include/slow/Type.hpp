#pragma once
#include <cstdint>

namespace slow {
    struct Vector2I {
        int32_t x;
        int32_t y;
    };
    struct Vector2U {
        uint32_t x;
        uint32_t y;
    };
    struct Vector2F {
        float x;
        float y;
    };
    
    struct Vector3I {
        int32_t x;
        int32_t y;
        int32_t z;
    };
    struct Vector3U {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };
    struct Vector3F {
        float x;
        float y;
        float z;
    };
    
    struct Vector4I {
        int32_t x;
        int32_t y;
        int32_t z;
        int32_t w;
    };
    struct Vector4U {
        uint32_t x;
        uint32_t y;
        uint32_t z;
        uint32_t w;
    };
    struct Vector4F {
        float x;
        float y;
        float z;
        float w;
    };
    
    struct Color3B {
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
    struct Color3F {
        float b;
        float g;
        float r;
    };
    
    struct Color4B {
        union {
            struct {
                uint8_t b;
                uint8_t g;
                uint8_t r;
                uint8_t a;
            };
            uint32_t bgra;
        };
    };
    struct Color4F {
        float b;
        float g;
        float r;
        float a;
    };
};
