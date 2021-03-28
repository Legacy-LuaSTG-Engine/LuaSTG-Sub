#pragma once

#include "Graphic/Type.h"
#include "Graphic/Object.h"

namespace slow::Graphic
{
    // resource
    
    enum class EResourceType
    {
        Buffer,
        Texture1D,
        Texture2D,
        Texture3D,
    };
    
    class IResource : public IObject
    {
    public:
        virtual EResourceType getType()=0;
        virtual handle_t getHandle()=0;
        virtual handle_t getViewHandle()=0;
    };
    
    // buffer
    
    class IBuffer : public IResource
    {
    public:
        virtual uint32_t getSize()=0;
    };
    
    class IVertexBuffer : public IBuffer
    {
    };
    
    class IIndexBuffer : public IBuffer
    {
    };
    
    // texture
    
    class ITexture : public IResource
    {
    public:
        virtual uint32_t getWidth()=0;
        virtual uint32_t getHeight()=0;
        virtual uint32_t getDepth()=0;
    };
    
    class ITexture2D : public ITexture
    {
    };
}
