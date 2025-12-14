#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IConstantBuffer : IReferenceCounted {
        virtual void* getNativeResource() = 0;
        virtual void* getNativeView() = 0;

        virtual uint32_t getSizeInBytes() = 0;
        virtual bool update(void const* data, uint32_t size_in_bytes) = 0;
    };
    CORE_INTERFACE_ID(IConstantBuffer, "caded37b-496e-532a-b6b9-f762aa659fc5")
}
