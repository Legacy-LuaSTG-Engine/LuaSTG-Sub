#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "pch.h"

namespace core {
    class VertexBuffer final :
        public implement::ReferenceCounted<IGraphicsBuffer>,
        public IGraphicsDeviceEventListener
    {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // IGraphicsBuffer

        void* getNativeResource() const override;
        void* getNativeView() const override;

        uint32_t getSizeInBytes() const override;
        uint32_t getStrideInBytes() const override;
        bool map(void** out_pointer, bool cycle) override;
        bool unmap() override;
        bool update(const void* data, uint32_t size, bool cycle) override;

        // VertexBuffer

        VertexBuffer();
        VertexBuffer(VertexBuffer const&) = delete;
        VertexBuffer(VertexBuffer&&) = delete;
        VertexBuffer& operator=(VertexBuffer const&) = delete;
        VertexBuffer& operator=(VertexBuffer&&) = delete;
        ~VertexBuffer();

        bool createResources();
        bool createResources(IGraphicsDevice* device, uint32_t size_in_bytes, uint32_t stride_in_bytes);

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Buffer> m_buffer;
        uint32_t m_size_in_bytes{};
        uint32_t m_stride_in_bytes{};
        bool m_initialized{};
    };
}

namespace core {
    class IndexBuffer final :
        public implement::ReferenceCounted<IGraphicsBuffer>,
        public IGraphicsDeviceEventListener
    {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // IGraphicsBuffer

        void* getNativeResource() const override;
        void* getNativeView() const override;

        uint32_t getSizeInBytes() const override;
        uint32_t getStrideInBytes() const override;
        bool map(void** out_pointer, bool cycle) override;
        bool unmap() override;
        bool update(const void* data, uint32_t size, bool cycle) override;

        // IndexBuffer

        IndexBuffer();
        IndexBuffer(IndexBuffer const&) = delete;
        IndexBuffer(IndexBuffer&&) = delete;
        IndexBuffer& operator=(IndexBuffer const&) = delete;
        IndexBuffer& operator=(IndexBuffer&&) = delete;
        ~IndexBuffer();

        bool createResources();
        bool createResources(IGraphicsDevice* device, uint32_t size_in_bytes, GraphicsFormat format);

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Buffer> m_buffer;
        uint32_t m_size_in_bytes{};
        GraphicsFormat m_format{};
        bool m_initialized{};
    };
}

namespace core {
    class ConstantBuffer final :
        public implement::ReferenceCounted<IGraphicsBuffer>,
        public IGraphicsDeviceEventListener
    {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // IGraphicsBuffer

        void* getNativeResource() const override;
        void* getNativeView() const override;

        uint32_t getSizeInBytes() const override;
        uint32_t getStrideInBytes() const override;
        bool map(void** out_pointer, bool cycle) override;
        bool unmap() override;
        bool update(const void* data, uint32_t size, bool cycle) override;

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
