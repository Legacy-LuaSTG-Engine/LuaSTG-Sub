#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"
#include "core/Rect.hpp"
#include "core/Data.hpp"
#include "core/ImmutableString.hpp"

namespace core {
    CORE_INTERFACE ITexture2D : IReferenceCounted {
        virtual void* getNativeResource() = 0;
        virtual void* getNativeView() = 0;

        virtual bool isDynamic() const noexcept = 0;
        virtual bool isPremultipliedAlpha() const noexcept = 0;
        virtual void setPremultipliedAlpha(bool v) = 0;
        virtual Vector2U getSize() const noexcept = 0;
        virtual bool setSize(Vector2U size) = 0;

        virtual bool uploadPixelData(RectU rc, void const* data, uint32_t pitch_in_bytes) = 0;
        virtual void setPixelData(IData* data) = 0;

        virtual bool saveToFile(StringView path) = 0;

        //virtual void setSamplerState(ISamplerState* p_sampler) = 0;
        // Might be nullptr
        //virtual ISamplerState* getSamplerState() const noexcept = 0;
    };
    CORE_INTERFACE_ID(ITexture2D, "5477054a-61c9-5071-9339-a9959e538a21")
}
