#include "d3d11/SecondarySwapChain.hpp"

namespace {
    using std::string_view_literals::operator ""sv;
}

namespace core {
    bool SecondarySwapChain::create(IDXGIFactory2* const factory, ID3D11Device* const device, ID2D1DeviceContext* const context, const Vector2U& size) {
        assert(factory != nullptr);
        assert(device != nullptr);
        assert(context != nullptr);
        if (size.x == 0 || size.y == 0) {
            assert(false); return false;
        }

        dxgi_factory = factory;
        d3d11_device = device;
        d3d11_device->GetImmediateContext(d3d11_device_context.put());
        d2d1_device_context = context;

        info.Width = size.x;
        info.Height = size.y;
        info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        info.SampleDesc.Count = 1;
        info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        info.BufferCount = 2;
        info.Scaling = DXGI_SCALING_STRETCH;
        info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        if (!win32::check_hresult_as_boolean(
            dxgi_factory->CreateSwapChainForComposition(d3d11_device.get(), &info, NULL, dxgi_swap_chain.put()),
            "IDXGIFactory2::CreateSwapChainForComposition"sv
        )) {
            return false;
        }

        return createRenderAttachment();
    }
    void SecondarySwapChain::destroy() {
        destroyRenderAttachment();
        dxgi_factory.reset();
        d3d11_device.reset();
        d3d11_device_context.reset();
        d2d1_device_context.reset();
        dxgi_swap_chain.reset();
    }
    bool SecondarySwapChain::setSize(const Vector2U& size) {
        if (size.x == 0 || size.y == 0) {
            assert(false); return false;
        }

        destroyRenderAttachment();

        info.Width = size.x;
        info.Height = size.y;

        if (!win32::check_hresult_as_boolean(
            dxgi_swap_chain->ResizeBuffers(info.BufferCount, info.Width, info.Height, info.Format, info.Flags),
            "IDXGISwapChain::ResizeBuffers"sv
        )) {
            return false;
        }

        return createRenderAttachment();
    }
    void SecondarySwapChain::clearRenderTarget() {
        assert(d3d11_device_context);
        assert(d3d11_rtv);
        if (d3d11_device_context && d3d11_rtv) {
            constexpr FLOAT solid_black[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
            d3d11_device_context->ClearRenderTargetView(d3d11_rtv.get(), solid_black);
        }
    }
    bool SecondarySwapChain::present() {
        if (!dxgi_swap_chain) {
            assert(false); return false;
        }

        const auto hr = win32::check_hresult(
            dxgi_swap_chain->Present(0, DXGI_PRESENT_ALLOW_TEARING),
            "IDXGISwapChain::Present"sv
        );
        if (hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED) {
            return false;
        }

        return true;
    }

    bool SecondarySwapChain::createRenderAttachment() {
        assert(d3d11_device);
        assert(d2d1_device_context);
        assert(dxgi_swap_chain);

        win32::com_ptr<ID3D11Texture2D> texture;
        if (!win32::check_hresult_as_boolean(
            dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(texture.put())),
            "IDXGISwapChain::GetBuffer (0, ID3D11Texture2D)"sv
        )) {
            return false;
        }

        if (!win32::check_hresult_as_boolean(
            d3d11_device->CreateRenderTargetView(texture.get(), NULL, d3d11_rtv.put()),
            "ID3D11Device::CreateRenderTargetView"sv
        )) {
            return false;
        }

        win32::com_ptr<IDXGISurface> surface;
        if (!win32::check_hresult_as_boolean(
            dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(surface.put())),
            "IDXGISwapChain::GetBuffer (0, IDXGISurface)"sv
        )) {
            return false;
        }

        D2D1_BITMAP_PROPERTIES1 bitmap_info{};
        bitmap_info.pixelFormat.format = info.Format;
        bitmap_info.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        bitmap_info.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

        if (!win32::check_hresult_as_boolean(
            d2d1_device_context->CreateBitmapFromDxgiSurface(surface.get(), &bitmap_info, d2d1_bitmap.put()),
            "ID2D1DeviceContext::CreateBitmapFromDxgiSurface"sv
        )) {
            return false;
        }

        return true;
    }
    void SecondarySwapChain::destroyRenderAttachment() {
        if (d3d11_device_context) {
            d3d11_device_context->ClearState();
            d3d11_device_context->Flush();
        }
        d3d11_rtv.reset();
        d2d1_bitmap.reset();
    }
}
