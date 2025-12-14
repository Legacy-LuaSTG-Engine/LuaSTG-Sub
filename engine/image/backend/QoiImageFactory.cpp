#include "backend/QoiImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "backend/Image.hpp"
#include "qoi.h"

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header{ "[core] [QoiImageFactory::createFromMemory]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };

    class ScopedMemory {
    public:
        ScopedMemory(void* const memory) noexcept : m_memory(memory) {}
        ~ScopedMemory() noexcept { if (m_memory) std::free(m_memory); }

        void detach() noexcept { m_memory = nullptr; }
    private:
        void* m_memory{};
    };
}

namespace core {
    bool QoiImageFactory::createFromMemory(LoggingBuffer& log, const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        if (data == nullptr) {
            L_ERROR("{} {} data is null pointer"sv, log_header, invalid_parameter_header);
            return false;
        }
        if (size_in_bytes == 0) {
            L_ERROR("{} {} size_in_bytes is 0"sv, log_header, invalid_parameter_header);
            return false;
        }
        if (output_image == nullptr) {
            L_ERROR("{} {} output_image is null pointer"sv, log_header, invalid_parameter_header);
            return false;
        }

        qoi_desc info{};
        const auto pixels = qoi_decode(data, static_cast<int>(size_in_bytes), &info, 4);
        if (pixels == nullptr) {
            L_ERROR("{} qoi_decode failed"sv, log_header);
            return false;
        }
        ScopedMemory scoped_pixels(pixels);

        if (info.width == 0 || info.width > 16384 || info.height == 0 || info.height > 16384) {
            L_ERROR("{} qoi image size too large ({}x{})"sv, log_header, info.width, info.height);
            return false;
        }

        ImageDescription description;
        description.size.x = info.width;
        description.size.y = info.height;
        description.format = ImageFormat::r8g8b8a8_normalized;
        description.color_space = info.colorspace == QOI_LINEAR
            ? ImageColorSpace::linear
            : ImageColorSpace::srgb_gamma_2_2;
        description.alpha_mode = ImageAlphaMode::straight;

        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initializeFromMemory(description, pixels, false)) {
            L_ERROR("{} Image::initializeFromMemory failed"sv, log_header);
            return false;
        }

        scoped_pixels.detach();
        *output_image = image.detach();
        return true;
    }
}
