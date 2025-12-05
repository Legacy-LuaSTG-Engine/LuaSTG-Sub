#ifdef LUASTG_IMAGE_PNG_ENABLE

#include "backend/PngImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "backend/Image.hpp"
#include "png.h"
#include <cassert>

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header{ "[core] [PngImageFactory::createFromMemory]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };

    class ScopedPngImage {
    public:
        explicit ScopedPngImage(png_image* const png) : m_png(png) {}
        ~ScopedPngImage() { png_image_free(m_png); }
    private:
        png_image* m_png{};
    };
}

namespace core {
    bool PngImageFactory::createFromMemory(LoggingBuffer& log, const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
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

        png_image png{};
        png.version = PNG_IMAGE_VERSION;
        const ScopedPngImage scoped_png(&png);
        if (!png_image_begin_read_from_memory(&png, data, size_in_bytes)) {
            L_ERROR("{} png image failed ({})"sv, log_header, png.message);
            return false;
        }
        if (PNG_IMAGE_FAILED(png)) {
            L_ERROR("{} png image failed ({})"sv, log_header, png.message);
            return false;
        }

        if (png.width <= 0 || png.width > 16384 || png.height <= 0 || png.height > 16384) {
            L_ERROR("{} png image size too large ({}x{})"sv, log_header, png.width, png.height);
            return false;
        }

        png.format = PNG_FORMAT_BGRA; // setup format

        ImageDescription description;
        description.size.x = png.width;
        description.size.y = png.height;
        description.format = ImageFormat::b8g8r8a8_normalized;
        description.color_space = ImageColorSpace::srgb_gamma_2_2;

        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initialize(description)) {
            L_ERROR("{} Image::initialize failed"sv, log_header);
            return false;
        }

        ScopedImageMappedBuffer buffer{};
        if (!image->createScopedMap(buffer)) {
            L_ERROR("{} Image::map failed"sv, log_header);
            return false;
        }

        if (!png_image_finish_read(
            &png,
            nullptr, // no background
            buffer.data, static_cast<png_int_32>(buffer.stride),
            nullptr // no color-map
        )) {
            L_ERROR("{} png_image_finish_read failed ({})"sv, log_header, png.message);
            return false;
        }
        if (PNG_IMAGE_FAILED(png)) {
            L_ERROR("{} png_image_finish_read failed ({})"sv, log_header, png.message);
            return false;
        }

        *output_image = image.detach();
        return true;
    }
}

#endif // LUASTG_IMAGE_PNG_ENABLE
