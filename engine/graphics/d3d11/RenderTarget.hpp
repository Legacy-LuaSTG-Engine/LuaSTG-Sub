#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    class RenderTarget final :
        public implement::ReferenceCounted<IRenderTarget>,
        public IGraphicsDeviceEventListener {
    public:
        // IRenderTarget

        void* getNativeView() const noexcept override { return m_view.get(); }
#ifdef LUASTG_ENABLE_DIRECT2D
        void* getNativeBitmap() const noexcept override { return m_bitmap.get(); }
#else
        void* getNativeBitmap() const noexcept override { return nullptr; }
#endif
        bool setSize(Vector2U size) override;
        ITexture2D* getTexture() const noexcept override { return m_texture.get(); }

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // RenderTarget

        RenderTarget();
        RenderTarget(RenderTarget const&) = delete;
        RenderTarget(RenderTarget&&) = delete;
        ~RenderTarget();

        RenderTarget& operator=(RenderTarget const&) = delete;
        RenderTarget& operator=(RenderTarget&&) = delete;

        ID3D11RenderTargetView* GetView() const noexcept { return m_view.get(); }

        bool initialize(IGraphicsDevice* device, Vector2U size);
        bool createResource();

    private:
        SmartReference<IGraphicsDevice> m_device;
        SmartReference<ITexture2D> m_texture;
        win32::com_ptr<ID3D11RenderTargetView> m_view;
#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<ID2D1Bitmap1> m_bitmap;
#endif
        bool m_initialized{ false };
    };
}
