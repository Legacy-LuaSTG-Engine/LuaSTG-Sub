#include "backend/WebpImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "core/Logger.hpp"
#include "backend/Image.hpp"
#include "webp/decode.h"
#include <cassert>

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header{ "[core] [WebpImageFactory::createFromMemory]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };
}

namespace core {
    bool WebpImageFactory::createFromMemory(const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        if (data == nullptr) {
            Logger::error("{} {} data is null pointer"sv, log_header, invalid_parameter_header);
            return false;
        }
        if (size_in_bytes == 0) {
            Logger::error("{} {} size_in_bytes is 0"sv, log_header, invalid_parameter_header);
            return false;
        }
        if (output_image == 0) {
            Logger::error("{} {} output_image is null pointer"sv, log_header, invalid_parameter_header);
            return false;
        }

        int width{}, height{};
        if (!WebPGetInfo(static_cast<const uint8_t*>(data), size_in_bytes, &width, &height)) {
            Logger::error("{} not a webp image"sv, log_header);
            return false;
        }

        if (width <= 0 || width > 16384 || height <= 0 || height > 16384) {
            Logger::error("{} webp image size ({}x{}) too large"sv, log_header, width, height);
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
            Logger::error("{} Image::initialize failed"sv, log_header);
            return false;
        }

        ImageMappedBuffer buffer{};
        if (!image->map(buffer)) {
            Logger::error("{} Image::map failed"sv, log_header);
            return false;
        }

        if (!WebPDecodeBGRAInto(
            static_cast<const uint8_t*>(data), size_in_bytes,
            static_cast<uint8_t*>(buffer.data), buffer.size, static_cast<int>(buffer.stride)
        )) {
            Logger::error("{} WebPDecodeBGRAInto failed"sv, log_header);
            image->unmap();
            return false;
        }

        image->unmap();
        *output_image = image.detach();
        return true;
    }
}
