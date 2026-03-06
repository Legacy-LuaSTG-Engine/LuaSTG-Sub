#pragma once
#include "core/Vector2.hpp"
#include "d3d11/pch.h"

namespace core {
    class SecondarySwapChain {
    public:
        IDXGISwapChain1* getSwapChain1() { return dxgi_swap_chain.get(); }
        ID2D1Bitmap1* getBitmap1() { return d2d1_bitmap.get(); }
        bool create(IDXGIFactory2* factory, ID3D11Device* device, ID2D1DeviceContext* context, const Vector2U& size);
        void destroy();
        bool setSize(const Vector2U& size);
        Vector2U getSize() const noexcept { return { info.Width, info.Height }; }
        void clearRenderTarget();
        bool present();

    private:
        bool createRenderAttachment();
        void destroyRenderAttachment();

        DXGI_SWAP_CHAIN_DESC1 info{};
        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        win32::com_ptr<ID3D11Device> d3d11_device;
        win32::com_ptr<ID3D11DeviceContext> d3d11_device_context;
        win32::com_ptr<ID2D1DeviceContext> d2d1_device_context;
        win32::com_ptr<IDXGISwapChain1> dxgi_swap_chain;
        win32::com_ptr<ID3D11RenderTargetView> d3d11_rtv;
        win32::com_ptr<ID2D1Bitmap1> d2d1_bitmap;
    };
}
