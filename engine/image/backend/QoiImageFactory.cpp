#include "backend/QoiImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "backend/Image.hpp"
#include "qoi.h"
#include <cassert>

namespace {
    class ScopedMemory {
    public:
        ScopedMemory(void* const memory) : m_memory(memory) {}
        ~ScopedMemory() { if (m_memory) std::free(m_memory); }
    private:
        void* m_memory{};
    };
}

namespace core {
    bool QoiImageFactory::createFromMemory(const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        qoi_desc info{};
        const auto pixels = qoi_decode(data, static_cast<int>(size_in_bytes), &info, 4);
        if (pixels == nullptr) {
            return false;
        }
        const ScopedMemory scoped_pixels(pixels);
        if (info.width == 0 || info.width > 16384 || info.height == 0 || info.height > 16384) {
            assert(false);
            return false;
        }

        ImageDescription description;
        description.size.x = info.width;
        description.size.y = info.height;
        description.format = ImageFormat::r8g8b8a8_normalized;
        description.color_space = info.colorspace == QOI_LINEAR
            ? ImageColorSpace::linear
            : ImageColorSpace::srgb_gamma_2_2;

        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initialize(description)) {
            assert(false);
            return false;
        }

        ScopedImageMappedBuffer buffer{};
        if (!image->createScopedMap(buffer)) {
            assert(false);
            return false;
        }

        const auto input_stride = sizeof(uint32_t) * info.width;
        auto input = static_cast<uint8_t*>(pixels);
        auto output = static_cast<uint8_t*>(buffer.data);
        for (uint32_t y = 0; y < info.height; y++) {
            std::memcpy(output, input, input_stride);
            input += input_stride;
            output += buffer.stride;
        }

        *output_image = image.detach();
        return true;
    }
}
