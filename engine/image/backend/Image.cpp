#include "backend/Image.hpp"
#include "core/SmartReference.hpp"
#include <cassert>
#include <algorithm>

namespace {
    constexpr auto _1_255 = 1.0f / 255.0f;
}

namespace core {
    // IImage

    void* Image::getBufferPointer() const noexcept { return m_pixels; }
    ImageFormat Image::getFormat() const noexcept { return m_format; }
    Vector2U Image::getSize() const noexcept { return m_size; }
    Vector4F Image::getPixel(const uint32_t x, const uint32_t y) const noexcept {
        if (x >= m_size.x || y >= m_size.y) {
            assert(false);
            return {};
        }
        if (m_format == ImageFormat::unknown || m_pixels == nullptr) {
            assert(false);
            return {};
        }
        switch (m_format) {
            case ImageFormat::r8g8b8a8: {
                const auto pixels = static_cast<const uint32_t*>(m_pixels);
                const auto pixel = pixels[m_size.x * y + x];
                const auto r = pixel & 0xff;
                const auto g = (pixel >> 8) & 0xff;
                const auto b = (pixel >> 16) & 0xff;
                const auto a = (pixel >> 24) & 0xff;
                return Vector4F(r * _1_255, g * _1_255, b * _1_255, a * _1_255);
            }
            case ImageFormat::b8g8r8a8: {
                const auto pixels = static_cast<const uint32_t*>(m_pixels);
                const auto pixel = pixels[m_size.x * y + x];
                const auto b = pixel & 0xff;
                const auto g = (pixel >> 8) & 0xff;
                const auto r = (pixel >> 16) & 0xff;
                const auto a = (pixel >> 24) & 0xff;
                return Vector4F(r * _1_255, g * _1_255, b * _1_255, a * _1_255);
            }
            case ImageFormat::r32g32b32a32: {
                const auto pixels = static_cast<const Vector4F*>(m_pixels);
                return pixels[m_size.x * y + x];
            }
            default: {
                assert(false);
                return {};
            }
        }
    }
    void Image::setPixel(const uint32_t x, const uint32_t y, const Vector4F& pixel) noexcept {
        if (x >= m_size.x || y >= m_size.y) {
            return;
        }
        if (m_read_only || m_format == ImageFormat::unknown || m_pixels == nullptr) {
            assert(false);
            return;
        }
        switch (m_format) {
            case ImageFormat::r8g8b8a8: {
                const auto r = static_cast<uint32_t>(std::clamp(255.0f * pixel.x, 0.0f, 255.0f) + 0.5f);
                const auto g = static_cast<uint32_t>(std::clamp(255.0f * pixel.y, 0.0f, 255.0f) + 0.5f);
                const auto b = static_cast<uint32_t>(std::clamp(255.0f * pixel.z, 0.0f, 255.0f) + 0.5f);
                const auto a = static_cast<uint32_t>(std::clamp(255.0f * pixel.w, 0.0f, 255.0f) + 0.5f);
                const auto rgba = (a << 24) | (b << 16) | (g << 8) | r;
                const auto pixels = static_cast<uint32_t*>(m_pixels);
                pixels[m_size.x * y + x] = rgba;
                break;
            }
            case ImageFormat::b8g8r8a8: {
                const auto r = static_cast<uint32_t>(std::clamp(255.0f * pixel.x, 0.0f, 255.0f) + 0.5f);
                const auto g = static_cast<uint32_t>(std::clamp(255.0f * pixel.y, 0.0f, 255.0f) + 0.5f);
                const auto b = static_cast<uint32_t>(std::clamp(255.0f * pixel.z, 0.0f, 255.0f) + 0.5f);
                const auto a = static_cast<uint32_t>(std::clamp(255.0f * pixel.w, 0.0f, 255.0f) + 0.5f);
                const auto bgra = (a << 24) | (r << 16) | (g << 8) | b;
                const auto pixels = static_cast<uint32_t*>(m_pixels);
                pixels[m_size.x * y + x] = bgra;
                break;
            }
            case ImageFormat::r32g32b32a32: {
                const auto pixels = static_cast<Vector4F*>(m_pixels);
                pixels[m_size.x * y + x] = pixel;
                break;
            }
            default: {
                assert(false);
                break;
            }
        }
    }
    bool Image::isReadOnly() const noexcept { return m_read_only; }
    void Image::setReadOnly() noexcept { m_read_only = true; }

    // Image

    Image::Image() = default;
    Image::~Image() {
        if (m_pixels != nullptr) {
            if (m_aligned) {
                _aligned_free(m_pixels);
            }
            else {
                std::free(m_pixels);
            }
        }
    }

    bool Image::initialize(const Vector2U size, const ImageFormat format) {
        if (size.x == 0 || size.x > 16384 || size.y == 0 || size.y > 16384) {
            assert(false);
            return false;
        }
    #define I(E) static_cast<int32_t>(E)
        if (I(format) <= I(ImageFormat::unknown) || I(format) >= I(ImageFormat::count)) {
            assert(false);
            return false;
        }
    #undef I
        if (m_pixels != nullptr) {
            if (m_aligned) {
                _aligned_free(m_pixels);
            }
            else {
                std::free(m_pixels);
            }
            m_pixels = nullptr;
        }
        m_size = size;
        m_format = format;
        m_read_only = false;
        m_aligned = true;
        switch (m_format) {
            case ImageFormat::r8g8b8a8:
            case ImageFormat::b8g8r8a8: {
                m_pixels = _aligned_malloc(sizeof(uint32_t) * m_size.x * m_size.y, alignof(uint32_t));
                break;
            }
            case ImageFormat::r32g32b32a32: {
                m_pixels = _aligned_malloc(sizeof(Vector4F) * m_size.x * m_size.y, alignof(Vector4F));
                break;
            }
            default: {
                assert(false);
                return false;
            }
        }
        return m_pixels != nullptr;
    }
    bool Image::initializeFromMemory(const Vector2U size, const ImageFormat format, void* const pixels, const bool aligned) {
        if (size.x == 0 || size.x > 16384 || size.y == 0 || size.y > 16384) {
            assert(false);
            return false;
        }
    #define I(E) static_cast<int32_t>(E)
        if (I(format) <= I(ImageFormat::unknown) || I(format) >= I(ImageFormat::count)) {
            assert(false);
            return false;
        }
    #undef I
        if (pixels == nullptr) {
            assert(false);
            return false;
        }
        if (m_pixels != nullptr) {
            if (m_aligned) {
                _aligned_free(m_pixels);
            }
            else {
                std::free(m_pixels);
            }
            m_pixels = nullptr;
        }
        m_pixels = pixels;
        m_size = size;
        m_format = format;
        m_read_only = false;
        m_aligned = aligned;
        return true;
    }
}

namespace core {
    bool ImageFactory::create(const Vector2U size, const ImageFormat format, IImage** const output_image) {
        if (size.x == 0 || size.x > 16384 || size.y == 0 || size.y > 16384) {
            assert(false);
            return false;
        }
    #define I(E) static_cast<int32_t>(E)
        if (I(format) <= I(ImageFormat::unknown) || I(format) >= I(ImageFormat::count)) {
            assert(false);
            return false;
        }
    #undef I
        if (!output_image) {
            assert(false);
            return false;
        }
        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initialize(size, format)) {
            assert(false);
            return false;
        }
        *output_image = image.detach();
        return true;
    }
}
