#ifdef LUASTG_IMAGE_WEBP_ENABLE

#include "backend/WebpImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "backend/Image.hpp"
#include "webp/decode.h"
#include <cassert>

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header{ "[core] [WebpImageFactory::createFromMemory]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };
}

namespace core {
    bool WebpImageFactory::createFromMemory(LoggingBuffer& log, const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
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

        int width{}, height{};
        if (!WebPGetInfo(static_cast<const uint8_t*>(data), size_in_bytes, &width, &height)) {
            L_ERROR("{} WebPGetInfo failed"sv, log_header);
            return false;
        }

        if (width <= 0 || width > 16384 || height <= 0 || height > 16384) {
            L_ERROR("{} webp image size too large ({}x{})"sv, log_header, width, height);
            return false;
        }

        ImageDescription description;
        description.size.x = static_cast<uint32_t>(width);
        description.size.y = static_cast<uint32_t>(height);
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

        if (!WebPDecodeBGRAInto(
            static_cast<const uint8_t*>(data), size_in_bytes,
            static_cast<uint8_t*>(buffer.data), buffer.size, static_cast<int>(buffer.stride)
        )) {
            L_ERROR("{} WebPDecodeBGRAInto failed"sv, log_header);
            return false;
        }

        *output_image = image.detach();
        return true;
    }
}

#endif // LUASTG_IMAGE_WEBP_ENABLE
