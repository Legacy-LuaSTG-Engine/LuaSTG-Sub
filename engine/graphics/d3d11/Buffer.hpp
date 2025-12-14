#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "win32/base.hpp"
#include <d3d11_4.h>

namespace core {
    class VertexBuffer final :
        public implement::ReferenceCounted<IVertexBuffer>,
        public IGraphicsDeviceEventListener
    {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // IVertexBuffer

        void* getNativeResource() override;
        void* getNativeView() override;
        uint32_t getSizeInBytes() override;
        bool map(uint32_t size_in_bytes, bool discard, void** out_pointer) override;
        bool unmap() override;

        // VertexBuffer

        VertexBuffer();
        VertexBuffer(VertexBuffer const&) = delete;
        VertexBuffer(VertexBuffer&&) = delete;
        VertexBuffer& operator=(VertexBuffer const&) = delete;
        VertexBuffer& operator=(VertexBuffer&&) = delete;
        ~VertexBuffer();

        bool createResources();
        bool createResources(IGraphicsDevice* device, uint32_t size_in_bytes);

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Buffer> m_buffer;
        uint32_t m_size_in_bytes{};
        bool m_initialized{};
    };
}

namespace core {
    class IndexBuffer final :
        public implement::ReferenceCounted<IIndexBuffer>,
        public IGraphicsDeviceEventListener
    {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // IIndexBuffer

        void* getNativeResource() override;
        void* getNativeView() override;
        uint32_t getSizeInBytes() override;
        bool map(uint32_t size_in_bytes, bool discard, void** out_pointer) override;
        bool unmap() override;

        // IndexBuffer

        IndexBuffer();
        IndexBuffer(IndexBuffer const&) = delete;
        IndexBuffer(IndexBuffer&&) = delete;
        IndexBuffer& operator=(IndexBuffer const&) = delete;
        IndexBuffer& operator=(IndexBuffer&&) = delete;
        ~IndexBuffer();

        bool createResources();
        bool createResources(IGraphicsDevice* device, uint32_t size_in_bytes);

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Buffer> m_buffer;
        uint32_t m_size_in_bytes{};
        bool m_initialized{};
    };
}

namespace core {
    class ConstantBuffer final :
        public implement::ReferenceCounted<IConstantBuffer>,
        public IGraphicsDeviceEventListener
    {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // IConstantBuffer

        void* getNativeResource() override;
        void* getNativeView() override;
        uint32_t getSizeInBytes() override;
        bool update(void const* data, uint32_t size_in_bytes) override;

        // ConstantBuffer

        ConstantBuffer();
        ConstantBuffer(ConstantBuffer const&) = delete;
        ConstantBuffer(ConstantBuffer&&) = delete;
        ConstantBuffer& operator=(ConstantBuffer const&) = delete;
        ConstantBuffer& operator=(ConstantBuffer&&) = delete;
        ~ConstantBuffer();

        bool createResources();
        bool createResources(IGraphicsDevice* device, uint32_t size_in_bytes);

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Buffer> m_buffer;
        uint32_t m_size_in_bytes{};
        bool m_initialized{};
    };
}
