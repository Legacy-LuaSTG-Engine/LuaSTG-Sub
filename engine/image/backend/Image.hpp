#pragma once
#include "core/Image.hpp"
#include "core/implement/ReferenceCounted.hpp"

namespace core {
    class Image final : public implement::ReferenceCounted<IImage> {
    public:
        // IImage

        bool map(ImageMappedBuffer& buffer) noexcept override;
        void unmap() noexcept override;
        const ImageDescription* getDescription() const noexcept override;
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

        bool initialize(const ImageDescription& description);
        bool initializeFromMemory(const ImageDescription& description, void* pixels, bool aligned);
        void destroyPixels();

    private:
        void* m_pixels{};
        ImageDescription m_description;
        bool m_read_only{};
        bool m_aligned{};
        bool m_mapped{};
    };
}
