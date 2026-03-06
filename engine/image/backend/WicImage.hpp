#pragma once
#include "core/Image.hpp"

namespace core {
    class WicImage {
    public:
        static bool createFromImage(IImage* image, void** output);
        static bool createFromImage(IImage* image, ImageMappedBuffer* buffer, void** output);
    };
}
