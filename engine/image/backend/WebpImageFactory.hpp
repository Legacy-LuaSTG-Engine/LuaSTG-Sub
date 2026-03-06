#pragma once
#ifdef LUASTG_IMAGE_WEBP_ENABLE

#include "core/Image.hpp"
#include "backend/LoggingBuffer.hpp"

namespace core {
    class WebpImageFactory {
    public:
        static bool createFromMemory(LoggingBuffer& log, const void* data, uint32_t size_in_bytes, IImage** output_image);
    };
}

#endif // LUASTG_IMAGE_WEBP_ENABLE
