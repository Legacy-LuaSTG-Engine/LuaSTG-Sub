#pragma once
#include "core/DepthStencilBuffer.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/GraphicsDevice.hpp"
#include "d3d11/pch.h"

namespace core {
    class DepthStencilBuffer final :
        public implement::ReferenceCounted<IDepthStencilBuffer>,
        public IGraphicsDeviceEventListener {
    public:
        // IDepthStencilBuffer

        void* getNativeResource() const noexcept override { return m_texture.get(); }
        void* getNativeView() const noexcept override { return m_view.get(); }

        bool setSize(Vector2U size) override;
        Vector2U getSize() const noexcept override { return m_size; }

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // DepthStencilBuffer

        DepthStencilBuffer();
        DepthStencilBuffer(DepthStencilBuffer const&) = delete;
        DepthStencilBuffer(DepthStencilBuffer&&) = delete;
        DepthStencilBuffer& operator=(DepthStencilBuffer const&) = delete;
        DepthStencilBuffer& operator=(DepthStencilBuffer&&) = delete;
        ~DepthStencilBuffer();

        bool initialize(IGraphicsDevice* device, Vector2U size);
        bool createResource();

    private:
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Texture2D> m_texture;
        win32::com_ptr<ID3D11DepthStencilView> m_view;
        Vector2U m_size{};
        bool m_initialized{ false };
    };
}
