#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IGraphicsBuffer : IReferenceCounted {
        virtual void* getNativeResource() const = 0;
        virtual void* getNativeView() const = 0;

        virtual uint32_t getSizeInBytes() const = 0;
        virtual uint32_t getStrideInBytes() const = 0;
        virtual bool map(void** out_pointer, bool cycle) = 0;
        virtual bool unmap() = 0;
        virtual bool update(const void* data, uint32_t size, bool cycle) = 0;
    };

    CORE_INTERFACE_ID(IGraphicsBuffer, "b76fab56-f2e9-5908-8d0b-a762275b2b7f")
}
