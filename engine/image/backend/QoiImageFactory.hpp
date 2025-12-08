#pragma once
#include "core/Image.hpp"
#include "backend/LoggingBuffer.hpp"

namespace core {
    class QoiImageFactory {
    public:
        static bool createFromMemory(LoggingBuffer& log, const void* data, uint32_t size_in_bytes, IImage** output_image);
    };
}
