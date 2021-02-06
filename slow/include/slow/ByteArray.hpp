#pragma once
#include <cstdint>
#include "slow/Object.hpp"

namespace slow
{
    struct ByteArray
    {
        virtual size_t size() = 0;
        virtual uint8_t* data() = 0;
        
        
    };
};
