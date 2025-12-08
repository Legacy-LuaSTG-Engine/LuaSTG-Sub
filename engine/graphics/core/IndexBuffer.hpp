#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IIndexBuffer : IReferenceCounted {
        virtual void* getNativeResource() = 0;
        virtual void* getNativeView() = 0;

        virtual uint32_t getSizeInBytes() = 0;
        virtual bool map(uint32_t size_in_bytes, bool discard, void** out_pointer) = 0;
        virtual bool unmap() = 0;
    };
    CORE_INTERFACE_ID(IIndexBuffer, "2ac72696-66cd-5f81-aa6f-36b62e88708a")
}
