#pragma once
#include <cstdint>

namespace slow
{
    struct Object
    {
        virtual int32_t retain() = 0;
        virtual int32_t release() = 0;
        
        virtual ~Object() {}
    };
};
