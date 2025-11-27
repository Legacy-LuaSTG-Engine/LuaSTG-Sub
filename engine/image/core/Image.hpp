#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"
#include "core/Vector4.hpp"
#include "core/ImmutableString.hpp"
#include "core/Data.hpp"

namespace core {
    enum class ImageFormat : int32_t {
        // Indicate that an error has occurred
        unknown,

        // [ r: u8, g: u8, b: u8, a: u8 ], or abgr: u32
        r8g8b8a8,

        // [ g: u8, g: u8, r: u8, a: u8 ], or argb: u32
        b8g8r8a8,

        // [ r: f32, g: f32, b: f32, a: f32 ], or rgba: float4
        r32g32b32a32,

        // Image format count
        count,
    };

    CORE_INTERFACE IImage : IReferenceCounted {
        // You should know what you are doing.
        virtual void* getBufferPointer() const noexcept = 0;

        // You should know what you are doing.
        virtual uint32_t getBufferStride() const noexcept = 0;

        // You should know what you are doing.
        virtual uint32_t getBufferSize() const noexcept = 0;

        // Get current image format, or return unknown if failed or not initialized.
        virtual ImageFormat getFormat() const noexcept = 0;

        // Get current image size.
        virtual Vector2U getSize() const noexcept = 0;

        // Get pixel at (x, y), value is always converted to float4.
        // If the (x, y) coordinates are out of range, return float4(0.0f, 0.0f, 0.0f, 0.0f).
        virtual Vector4F getPixel(uint32_t x, uint32_t y) const noexcept = 0;

        // Set pixel at (x, y), value will be automatically converted to the image format.
        // If the (x, y) coordinates are out of range, nothing will happen.
        virtual void setPixel(uint32_t x, uint32_t y, const Vector4F& pixel) noexcept = 0;

        // Determine whether the current image is read-only.
        // The image can be manually set to read-only via setReadOnly.
        virtual bool isReadOnly() const noexcept = 0;

        // Sets the image to read-only state.
        virtual void setReadOnly() noexcept = 0;
    };
    CORE_INTERFACE_ID(IImage, "5e4c12e0-e094-5346-b129-b9ddbb881373")

    class ImageFactory {
    public:
        // Create a modifiable image.
        static bool create(Vector2U size, ImageFormat format, IImage** output_image);

        // Load image from file.
        // See: createFromMemory.
        static bool createFromFile(StringView path, IImage** output_image);
        
        // Load image from memory.
        // Supported image container formats:
        // - bmp
        // - jpeg
        // - png
        // - webp
        // - qoi
        static bool createFromMemory(const void* data, uint32_t size_in_bytes, IImage** output_image);

        // Load image from IData object.
        // See: createFromMemory.
        static bool createFromData(IData* data, IImage** output_image);
    };
}
