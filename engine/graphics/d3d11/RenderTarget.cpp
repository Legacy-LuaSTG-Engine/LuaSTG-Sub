#include "d3d11/RenderTarget.hpp"
#include "core/Logger.hpp"
#include "d3d11/Texture2D.hpp"

namespace core {
    // IGraphicsDeviceEventListener

    void RenderTarget::onGraphicsDeviceCreate() {
        if (m_initialized) {
            // 这里不能直接调用 texture 的 onDeviceCreate，因为要判断创建是否成功
            if (static_cast<Texture2D*>(m_texture.get())->createResource()) {
                createResource();
            }
        }
    }
    void RenderTarget::onGraphicsDeviceDestroy() {
        m_view.reset();
#ifdef LUASTG_ENABLE_DIRECT2D
        m_bitmap.reset();
#endif
        static_cast<Texture2D*>(m_texture.get())->onGraphicsDeviceDestroy();
    }

    // RenderTarget

    RenderTarget::RenderTarget() = default;
    RenderTarget::~RenderTarget() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool RenderTarget::setSize(Vector2U const size) {
        m_view.reset();
#ifdef LUASTG_ENABLE_DIRECT2D
        m_bitmap.reset();
#endif
        if (!m_texture->setSize(size)) {
            return false;
        }
        return createResource();
    }

    bool RenderTarget::initialize(IGraphicsDevice* const device, Vector2U const size) {
        assert(device);
        assert(size.x > 0 && size.y > 0);
        m_device = device;
        m_texture.attach(new Texture2D);
        if (!static_cast<Texture2D*>(m_texture.get())->initialize(device, size, true)) {
            return false;
        }
        if (!createResource()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
    bool RenderTarget::createResource() {
        HRESULT hr = S_OK;

        const auto d3d11_device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (!d3d11_device)
            return false;
        win32::com_ptr<ID3D11DeviceContext> d3d11_devctx;
        d3d11_device->GetImmediateContext(d3d11_devctx.put());
        if (!d3d11_devctx)
            return false;
#ifdef LUASTG_ENABLE_DIRECT2D
        const auto d2d1_device_context = static_cast<ID2D1DeviceContext*>(m_device->getNativeRendererHandle());
        if (!d2d1_device_context)
            return false;
#endif

        // 获取纹理资源信息

        const auto texture = static_cast<ID3D11Texture2D*>(m_texture->getNativeResource());
        assert(texture != nullptr);
        D3D11_TEXTURE2D_DESC tex2ddef = {};
        texture->GetDesc(&tex2ddef);

        // 创建渲染目标视图

        D3D11_RENDER_TARGET_VIEW_DESC rtvdef = {
            .Format = tex2ddef.Format,
            // TODO: sRGB
            //.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
            .Texture2D = D3D11_TEX2D_RTV{.MipSlice = 0,},
        };
        hr = gHR = d3d11_device->CreateRenderTargetView(texture, &rtvdef, m_view.put());
        if (FAILED(hr)) {
            Logger::error("Windows API failed: ID3D11Device::CreateRenderTargetView");
            return false;
        }
        M_D3D_SET_DEBUG_NAME(m_view.get(), "RenderTarget_D3D11::d3d11_rtv");

        // 创建D2D1位图

#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<IDXGISurface> dxgi_surface;
        hr = gHR = texture->QueryInterface(dxgi_surface.put());
        if (FAILED(hr)) {
            Logger::error("Windows API failed: ID3D11Texture2D::QueryInterface -> IDXGISurface");
            return false;
        }

        D2D1_BITMAP_PROPERTIES1 bitmap_info = {
            .pixelFormat = {
                .format = DXGI_FORMAT_B8G8R8A8_UNORM,
                .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED,
            },
            .dpiX = 0.0f,
            .dpiY = 0.0f,
            .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET,
            .colorContext = nullptr,
        };
        hr = gHR = d2d1_device_context->CreateBitmapFromDxgiSurface(dxgi_surface.get(), &bitmap_info, m_bitmap.put());
        if (FAILED(hr)) {
            Logger::error("Windows API failed: ID3D11DeviceContext::CreateBitmapFromDxgiSurface");
            return false;
        }
#endif

        return true;
    }
}

#include "d3d11/GraphicsDevice.hpp"

namespace core {
    bool GraphicsDevice::createRenderTarget(Vector2U const size, IRenderTarget** const pp_rt) {
        *pp_rt = nullptr;
        SmartReference<RenderTarget> buffer;
        buffer.attach(new RenderTarget);
        if (!buffer->initialize(this, size)) {
            return false;
        }
        *pp_rt = buffer.detach();
        return true;
    }
}
