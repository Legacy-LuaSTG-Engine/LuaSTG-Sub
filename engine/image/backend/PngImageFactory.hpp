#pragma once
#ifdef LUASTG_IMAGE_PNG_ENABLE

#include "core/Image.hpp"

namespace core {
    class PngImageFactory {
    public:
        static bool createFromMemory(const void* data, uint32_t size_in_bytes, IImage** output_image);
    };
}

#endif // LUASTG_IMAGE_PNG_ENABLE
