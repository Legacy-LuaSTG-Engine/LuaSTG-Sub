#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
    CORE_INTERFACE IVertexBuffer : IReferenceCounted {
        virtual void* getNativeResource() = 0;
        virtual void* getNativeView() = 0;

        virtual uint32_t getSizeInBytes() = 0;
        virtual bool map(uint32_t size_in_bytes, bool discard, void** out_pointer) = 0;
        virtual bool unmap() = 0;
    };
    CORE_INTERFACE_ID(IVertexBuffer, "5ae9e06f-92b3-5816-a677-7bf2f708b778")
}
