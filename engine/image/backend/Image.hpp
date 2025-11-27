#pragma once
#include "core/Image.hpp"
#include "core/implement/ReferenceCounted.hpp"

namespace core {
    class Image final : public implement::ReferenceCounted<IImage> {
    public:
        // IImage

        void* getBufferPointer() const noexcept override;
        ImageFormat getFormat() const noexcept override;
        Vector2U getSize() const noexcept override;
        Vector4F getPixel(uint32_t x, uint32_t y) const noexcept override;
        void setPixel(uint32_t x, uint32_t y, const Vector4F& pixel) noexcept override;
        bool isReadOnly() const noexcept override;
        void setReadOnly() noexcept override;

        // Image

        Image();
        Image(Image const&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image const&) = delete;
        Image& operator=(Image&&) = delete;
        ~Image();

        bool initialize(Vector2U size, ImageFormat format);
        bool initializeFromMemory(Vector2U size, ImageFormat format, void* pixels, bool aligned);

    private:
        void* m_pixels{};
        Vector2U m_size{};
        ImageFormat m_format{};
        bool m_read_only{};
        bool m_aligned{};
    };
}
