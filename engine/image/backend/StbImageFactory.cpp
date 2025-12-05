#include "backend/StbImageFactory.hpp"
#include "core/SmartReference.hpp"
#include "core/Logger.hpp"
#include "backend/Image.hpp"
#include "stb_image.h"
#include <cassert>

namespace {
    using std::string_view_literals::operator ""sv;

    constexpr auto log_header{ "[core] [StbImageFactory::createFromMemory]"sv };
    constexpr auto invalid_parameter_header{ "invalid parameter:"sv };

    class ScopedMemory {
    public:
        explicit ScopedMemory(stbi_uc* const memory) noexcept : m_memory(memory) {}
        ~ScopedMemory() noexcept { if (m_memory) stbi_image_free(m_memory); }

        void detach() noexcept { m_memory = nullptr; }
    private:
        stbi_uc* m_memory{};
    };

    std::string_view getFailureReason() {
        const auto message = stbi_failure_reason();
        return message ? std::string_view(message) : "unknown"sv;
    }
}

namespace core {
    bool StbImageFactory::createFromMemory(const void* const data, const uint32_t size_in_bytes, IImage** const output_image) {
        if (data == nullptr) {
            Logger::error("{} {} data is null pointer"sv, log_header, invalid_parameter_header);
            return false;
        }
        if (size_in_bytes == 0) {
            Logger::error("{} {} size_in_bytes is 0"sv, log_header, invalid_parameter_header);
            return false;
        }
        if (output_image == nullptr) {
            Logger::error("{} {} output_image is null pointer"sv, log_header, invalid_parameter_header);
            return false;
        }

        stbi_set_unpremultiply_on_load_thread(true);
        stbi_convert_iphone_png_to_rgb_thread(true);
        stbi_set_flip_vertically_on_load_thread(false);

        int width{}, height{}, channels{};
        const auto pixels = stbi_load_from_memory(
            static_cast<const uint8_t*>(data), static_cast<int>(size_in_bytes),
            &width, &height, &channels,
            4 // RGBA
        );
        if (pixels == nullptr) {
            Logger::error("{} stbi_load_from_memory failed ({})"sv, log_header, getFailureReason());
            return false;
        }
        ScopedMemory scoped_memory(pixels);

        if (width <= 0 || width > 16384 || height <= 0 || height > 16384) {
            Logger::error("{} image size ({}x{}) too large"sv, log_header, width, height);
            return false;
        }

        ImageDescription description;
        description.size.x = static_cast<uint32_t>(width);
        description.size.y = static_cast<uint32_t>(height);
        description.format = ImageFormat::r8g8b8a8_normalized;
        description.color_space = ImageColorSpace::srgb_gamma_2_2;

        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initializeFromMemory(description, pixels, false)) {
            Logger::error("{} Image::initializeFromMemory failed"sv, log_header);
            return false;
        }

        scoped_memory.detach();
        *output_image = image.detach();
        return true;
    }
}
