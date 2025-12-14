#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/ImmutableString.hpp"

namespace core {
    struct IGraphicsDevice;

    enum class GraphicsResourceType : int32_t {
        unknown = 0,

        buffer = 0x1,
        vertex_buffer = 0x3,
        index_buffer = 0x5,
        constant_bufer = 0x9,

        texture = 0x100,
        line_texture = 0x300,
        surface_texture = 0x500,
        volume_texture = 0x900,
        cube_texture = 0x1100,
    };

    CORE_INTERFACE IGraphicsResource : IReferenceCounted {
        virtual void getDevice(IGraphicsDevice** output) = 0;
        virtual void setDebugName(StringView name) = 0;
        virtual void* getNativeResource() = 0;
        virtual void* getNativeView() = 0;
        virtual GraphicsResourceType getType() = 0;
    };
    CORE_INTERFACE_ID(IGraphicsResource, "8f558a3d-57e1-518c-b669-71d382320d6f");

    CORE_INTERFACE IGraphicsBuffer : IGraphicsResource {
    };
    CORE_INTERFACE_ID(IGraphicsBuffer, "b76fab56-f2e9-5908-8d0b-a762275b2b7f");

    CORE_INTERFACE IGraphicsTexture : IGraphicsResource {
    };
    CORE_INTERFACE_ID(IGraphicsTexture, "e0c57677-7648-5bca-9be1-f3bf448363e6");
}
