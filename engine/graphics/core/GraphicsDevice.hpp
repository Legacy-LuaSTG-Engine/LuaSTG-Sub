#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/VertexBuffer.hpp"
#include "core/IndexBuffer.hpp"
#include "core/ConstantBuffer.hpp"

namespace core {
    CORE_INTERFACE IGraphicsDeviceEventListener {
        virtual void onGraphicsDeviceCreate() = 0;
        virtual void onGraphicsDeviceDestroy() = 0;
    };

    CORE_INTERFACE IGraphicsDevice : IReferenceCounted {
        virtual void* getNativeDevice() = 0;

        virtual void addEventListener(IGraphicsDeviceEventListener* listener) = 0;
        virtual void removeEventListener(IGraphicsDeviceEventListener* listener) = 0;

        virtual bool createVertexBuffer(uint32_t size_in_bytes, IVertexBuffer** output_buffer) = 0;
        virtual bool createIndexBuffer(uint32_t size_in_bytes, IIndexBuffer** output_buffer) = 0;
        virtual bool createConstantBuffer(uint32_t size_in_bytes, IConstantBuffer** output_buffer) = 0;
    };
    CORE_INTERFACE_ID(IGraphicsDevice, "17b76b63-ceb6-5f87-aa5f-366e89d7176e")
}
