#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"

namespace core {
    CORE_INTERFACE IDepthStencilBuffer : IReferenceCounted {
        virtual void* getNativeResource() const noexcept = 0;
        virtual void* getNativeView() const noexcept = 0;

        virtual Vector2U getSize() const noexcept = 0;
        virtual bool setSize(Vector2U size) = 0;
    };

    CORE_INTERFACE_ID(IDepthStencilBuffer, "2c92426c-9703-57aa-88e6-073957b0d92c");
}
