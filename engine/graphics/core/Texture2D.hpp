#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/ImmutableString.hpp"
#include "core/Image.hpp"
#include "core/GraphicsFormat.hpp"
#include "core/GraphicsSampler.hpp"

namespace core {
    enum class TextureBindFlag {
        shader_resource = 0x8,
        render_target = 0x20,
        depth_stencil_buffer = 0x40,
    };

    struct TextureInfo {
        GraphicsFormat format;
        TextureBindFlag bind_flags;
        uint32_t width;
        uint32_t height;
        uint32_t mipmap_level;
    };

    CORE_INTERFACE ITexture2D : IReferenceCounted {
        virtual void* getNativeResource() const noexcept = 0;
        virtual void* getNativeView() const noexcept = 0;

        virtual bool isDynamic() const noexcept = 0;
        virtual bool isPremultipliedAlpha() const noexcept = 0;
        virtual void setPremultipliedAlpha(bool v) = 0;
        virtual Vector2U getSize() const noexcept = 0;

        virtual bool resize(Vector2U size) = 0;
        virtual bool update(RectU rect, void const* data, uint32_t row_pitch_in_bytes) = 0;
        virtual void setImage(IImage* image) = 0;

        virtual bool saveToFile(StringView path) = 0;

        virtual void setSamplerState(IGraphicsSampler* sampler) = 0;
        virtual IGraphicsSampler* getSamplerState() const noexcept = 0;
    };

    CORE_INTERFACE_ID(ITexture2D, "5477054a-61c9-5071-9339-a9959e538a21")
}
