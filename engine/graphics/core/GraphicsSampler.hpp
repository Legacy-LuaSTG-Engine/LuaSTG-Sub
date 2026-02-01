#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IGraphicsSampler : IReferenceCounted {
        virtual void* getNativeHandle() const noexcept = 0;
    };

    CORE_INTERFACE_ID(IGraphicsSampler, "446fb2a1-0bb3-50c1-bb64-4de769036200");
}
