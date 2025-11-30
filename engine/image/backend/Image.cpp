#include "backend/Image.hpp"
#include "core/SmartReference.hpp"
#include <cassert>
#include <algorithm>
#include <DirectXPackedVector.h>

namespace {
    constexpr auto _1_255 = 1.0f / 255.0f;

    bool validateImageDescription(const core::ImageDescription& description) {
        if (description.size.x == 0 || description.size.x > 16384 || description.size.y == 0 || description.size.y > 16384) {
            assert(false);
            return false;
        }
    #define I(E) static_cast<int32_t>(E)
        if (I(description.format) <= I(core::ImageFormat::unknown) || I(description.format) >= I(core::ImageFormat::count)) {
            assert(false);
            return false;
        }
        if (I(description.color_space) <= I(core::ImageColorSpace::unknown) || I(description.color_space) >= I(core::ImageColorSpace::count)) {
            assert(false);
            return false;
        }
    #undef I
        if (description.color_space == core::ImageColorSpace::srgb_gamma_2_2) {
            switch (description.format) {
                case core::ImageFormat::r8g8b8a8_normalized:
                case core::ImageFormat::b8g8r8a8_normalized: {
                    break;
                }
                default: {
                    assert(false);
                    return false;
                }
            }
        }
        return true;
    }

    uint32_t getImageFormatPixelSize(const core::ImageFormat format) {
        switch (format) {
            case core::ImageFormat::r8g8b8a8_normalized:
            case core::ImageFormat::b8g8r8a8_normalized: {
                return static_cast<uint32_t>(sizeof(DirectX::PackedVector::XMCOLOR));
            }
            case core::ImageFormat::r16g16b16a16_float: {
                return static_cast<uint32_t>(sizeof(DirectX::PackedVector::XMHALF4));
            }
            case core::ImageFormat::r32g32b32a32_float: {
                return static_cast<uint32_t>(sizeof(DirectX::XMFLOAT4));
            }
            default: {
                assert(false);
                return 0;
            }
        }
    }

    uint32_t getImageFormatAlignment(const core::ImageFormat format) {
        switch (format) {
            case core::ImageFormat::r8g8b8a8_normalized:
            case core::ImageFormat::b8g8r8a8_normalized: {
                return static_cast<uint32_t>(alignof(DirectX::PackedVector::XMCOLOR));
            }
            case core::ImageFormat::r16g16b16a16_float: {
                return static_cast<uint32_t>(alignof(DirectX::PackedVector::XMHALF4));
            }
            case core::ImageFormat::r32g32b32a32_float: {
                return static_cast<uint32_t>(alignof(DirectX::XMFLOAT4));
            }
            default: {
                assert(false);
                return 0;
            }
        }
    }

    size_t getOffset(const core::ImageDescription& description, const uint32_t x, const uint32_t y) {
        return description.size.x * y + x;
    }
}

namespace core {
    // IImage

    bool Image::map(ImageMappedBuffer& buffer) noexcept {
        if (m_mapped) {
            assert(false);
            return false;
        }
        buffer.data = m_pixels;
        buffer.stride = m_description.size.x * getImageFormatPixelSize(m_description.format);
        buffer.size = m_description.size.y * buffer.stride;
        return true;
    }
    void Image::unmap() noexcept { m_mapped = false; }
    const ImageDescription* Image::getDescription() const noexcept { return &m_description; }
    Vector4F Image::getPixel(const uint32_t x, const uint32_t y) const noexcept {
        if (x >= m_description.size.x || y >= m_description.size.y) {
            assert(false);
            return {};
        }
        switch (m_description.format) {
            case ImageFormat::r8g8b8a8_normalized: {
                DirectX::XMFLOAT4 pixel;
                DirectX::XMStoreFloat4(&pixel, DirectX::PackedVector::XMLoadColor(
                    &static_cast<const DirectX::PackedVector::XMCOLOR*>(m_pixels)[getOffset(m_description, x, y)]
                ));
                return Vector4F(pixel.z, pixel.y, pixel.x, pixel.w);
            }
            case ImageFormat::b8g8r8a8_normalized: {
                DirectX::XMFLOAT4 pixel;
                DirectX::XMStoreFloat4(&pixel, DirectX::PackedVector::XMLoadColor(
                    &static_cast<const DirectX::PackedVector::XMCOLOR*>(m_pixels)[getOffset(m_description, x, y)]
                ));
                return *reinterpret_cast<Vector4F*>(&pixel);
            }
            case ImageFormat::r16g16b16a16_float: {
                DirectX::XMFLOAT4 pixel;
                DirectX::XMStoreFloat4(&pixel, DirectX::PackedVector::XMLoadHalf4(
                    &static_cast<const DirectX::PackedVector::XMHALF4*>(m_pixels)[getOffset(m_description, x, y)]
                ));
                return *reinterpret_cast<Vector4F*>(&pixel);
            }
            case ImageFormat::r32g32b32a32_float: {
                return static_cast<const Vector4F*>(m_pixels)[getOffset(m_description, x, y)];
            }
            default: {
                assert(false);
                return {};
            }
        }
    }
    void Image::setPixel(const uint32_t x, const uint32_t y, const Vector4F& pixel) noexcept {
        if (x >= m_description.size.x || y >= m_description.size.y) {
            return;
        }
        if (m_read_only) {
            assert(false);
            return;
        }
        switch (m_description.format) {
            case ImageFormat::r8g8b8a8_normalized: {
                const DirectX::XMFLOAT4 rgba(pixel.z, pixel.y, pixel.x, pixel.w);
                DirectX::PackedVector::XMStoreColor(
                    &static_cast<DirectX::PackedVector::XMCOLOR*>(m_pixels)[getOffset(m_description, x, y)],
                    DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&rgba))
                );
                break;
            }
            case ImageFormat::b8g8r8a8_normalized: {
                DirectX::PackedVector::XMStoreColor(
                    &static_cast<DirectX::PackedVector::XMCOLOR*>(m_pixels)[getOffset(m_description, x, y)],
                    DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&pixel))
                );
                break;
            }
            case ImageFormat::r16g16b16a16_float: {
                DirectX::PackedVector::XMStoreHalf4(
                    &static_cast<DirectX::PackedVector::XMHALF4*>(m_pixels)[getOffset(m_description, x, y)],
                    DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&pixel))
                );
                break;
            }
            case ImageFormat::r32g32b32a32_float: {
                static_cast<Vector4F*>(m_pixels)[getOffset(m_description, x, y)] = pixel;
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
    Image::~Image() { destroyPixels(); }

    bool Image::initialize(const ImageDescription& description) {
        if (!validateImageDescription(description)) {
            return false;
        }
        destroyPixels();
        m_pixels = nullptr;
        m_description = description;
        m_read_only = false;
        m_aligned = true;
        m_pixels = _aligned_malloc(
            getImageFormatPixelSize(m_description.format) * m_description.size.x * m_description.size.y,
            getImageFormatAlignment(m_description.format)
        );
        return m_pixels != nullptr;
    }
    bool Image::initializeFromMemory(const ImageDescription& description, void* const pixels, const bool aligned) {
        if (!validateImageDescription(description)) {
            return false;
        }
        if (pixels == nullptr) {
            assert(false);
            return false;
        }
        destroyPixels();
        m_pixels = pixels;
        m_description = description;
        m_read_only = false;
        m_aligned = aligned;
        return true;
    }
    void Image::destroyPixels() {
        if (m_pixels != nullptr) {
            if (m_aligned) {
                _aligned_free(m_pixels);
            }
            else {
                std::free(m_pixels);
            }
            m_pixels = nullptr;
        }
    }
}

namespace core {
    bool ImageFactory::create(const ImageDescription& description, IImage** const output_image) {
        if (!validateImageDescription(description)) {
            return false;
        }
        if (!output_image) {
            assert(false);
            return false;
        }
        SmartReference<Image> image;
        image.attach(new Image());
        if (!image->initialize(description)) {
            assert(false);
            return false;
        }
        *output_image = image.detach();
        return true;
    }
}
