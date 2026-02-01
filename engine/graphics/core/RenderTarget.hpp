#pragma once
#include "core/Texture2D.hpp"

namespace core {
    CORE_INTERFACE IRenderTarget : IReferenceCounted {
        virtual void* getNativeView() const noexcept = 0;
        // (Direct2D)
        virtual void* getNativeBitmap() const noexcept = 0;

        virtual bool setSize(Vector2U size) = 0;
        virtual ITexture2D* getTexture() const noexcept = 0;
    };

    CORE_INTERFACE_ID(IRenderTarget, "2753f336-07b6-5e69-95b1-46f1125531fa");
}
