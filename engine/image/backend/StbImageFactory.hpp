#pragma once
#ifdef LUASTG_IMAGE_STB_ENABLE

#include "core/Image.hpp"
#include "backend/LoggingBuffer.hpp"

namespace core {
    class StbImageFactory {
    public:
        static bool createFromMemory(LoggingBuffer& log, const void* data, uint32_t size_in_bytes, IImage** output_image);
    };
}

#endif // LUASTG_IMAGE_STB_ENABLE
