#include "backend/QoiImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "backend/Image.hpp"
#include "qoi.h"
#include <cassert>

namespace core {
    bool QoiImageFactory::createFromMemory(const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        qoi_desc info{};
        const auto pixels = qoi_decode(data, static_cast<int>(size_in_bytes), &info, 4);
        if (pixels == nullptr) {
            return false;
        }
        if (info.width == 0 || info.width > 16384 || info.height == 0 || info.height > 16384) {
            assert(false);
            std::free(pixels);
            return false;
        }
        SmartReference<Image> image;
        image.attach(new Image());
        [[maybe_unused]] const auto result = image->initializeFromMemory(Vector2U(info.width, info.height), ImageFormat::r8g8b8a8, pixels, false);
        assert(result);
        *output_image = image.detach();
        return true;
    }
}
