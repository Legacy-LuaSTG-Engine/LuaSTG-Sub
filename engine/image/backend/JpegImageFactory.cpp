#ifdef LUASTG_IMAGE_JPEG_ENABLE

#include "backend/JpegImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "core/Logger.hpp"
#include "backend/Image.hpp"
#include "turbojpeg.h"
#include <cassert>

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header{ "[core] [JpegImageFactory::createFromMemory]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };

    class ScopedJpegHandle {
    public:
        explicit ScopedJpegHandle(tjhandle const jpeg) : m_jpeg(jpeg) {}
        ~ScopedJpegHandle() { tj3Destroy(m_jpeg); }
    private:
        tjhandle m_jpeg{};
    };

    std::string_view getErrorMessage(const tjhandle handle) {
        const auto message = tj3GetErrorStr(handle);
        return message ? std::string_view(message) : "unknown"sv;
    }
}

namespace core {
    bool JpegImageFactory::createFromMemory(const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
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

        const tjhandle jpeg = tj3Init(TJINIT_DECOMPRESS);
        if (jpeg == nullptr) {
            Logger::error("{} tj3Init failed ({})"sv, log_header, getErrorMessage(jpeg));
            return false;
        }
        const ScopedJpegHandle scoped_jpeg(jpeg);

        if (tj3DecompressHeader(jpeg, static_cast<const uint8_t*>(data), size_in_bytes) != 0) {
            Logger::error("{} tj3DecompressHeader failed ({})"sv, log_header, getErrorMessage(jpeg));
            return false;
        }

        const auto width = tj3Get(jpeg, TJPARAM_JPEGWIDTH);
        const auto height = tj3Get(jpeg, TJPARAM_JPEGHEIGHT);
        if (width <= 0 || width > 16384 || height <= 0 || height > 16384) {
            Logger::error("{} jpeg image size ({}x{}) too large"sv, log_header, width, height);
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

        ScopedImageMappedBuffer buffer{};
        if (!image->createScopedMap(buffer)) {
            Logger::error("{} Image::map failed"sv, log_header);
            return false;
        }

        if (tj3Decompress8(
            jpeg,
            static_cast<const uint8_t*>(data), size_in_bytes,
            static_cast<uint8_t*>(buffer.data), static_cast<int>(buffer.stride),
            TJPF_BGRA
        ) != 0) {
            Logger::error("{} tj3Decompress8 failed ({})"sv, log_header, getErrorMessage(jpeg));
            return false;
        }

        *output_image = image.detach();
        return true;
    }
}

#endif // LUASTG_IMAGE_JPEG_ENABLE
