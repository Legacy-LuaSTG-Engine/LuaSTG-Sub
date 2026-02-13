#include "d3d11/DepthStencilBuffer.hpp"
#include "core/Logger.hpp"

namespace core {
    // IDepthStencilBuffer

    bool DepthStencilBuffer::setSize(Vector2U const size) {
        m_texture.reset();
        m_view.reset();
        m_size = size;
        return createResource();
    }

    // IGraphicsDeviceEventListener

    void DepthStencilBuffer::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResource();
        }
    }
    void DepthStencilBuffer::onGraphicsDeviceDestroy() {
        m_texture.reset();
        m_view.reset();
    }

    // DepthStencilBuffer

    DepthStencilBuffer::DepthStencilBuffer() = default;
    DepthStencilBuffer::~DepthStencilBuffer() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool DepthStencilBuffer::initialize(IGraphicsDevice* const device, const Vector2U size) {
        assert(device);
        assert(size.x > 0 && size.y > 0);
        m_device = device;
        m_size = size;
        if (!createResource()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
    bool DepthStencilBuffer::createResource() {
        HRESULT hr = S_OK;

        const auto d3d11_device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (!d3d11_device)
            return false;
        win32::com_ptr<ID3D11DeviceContext> d3d11_devctx;
        d3d11_device->GetImmediateContext(d3d11_devctx.put());
        if (!d3d11_devctx)
            return false;

        D3D11_TEXTURE2D_DESC tex2ddef = {
            .Width = m_size.x,
            .Height = m_size.y,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = DXGI_SAMPLE_DESC{.Count = 1,.Quality = 0,},
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        hr = gHR = d3d11_device->CreateTexture2D(&tex2ddef, nullptr, m_texture.put());
        if (FAILED(hr)) {
            Logger::error("Windows API failed: ID3D11Device::CreateTexture2D");
            return false;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvdef = {
            .Format = tex2ddef.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = D3D11_TEX2D_DSV{.MipSlice = 0,},
        };
        hr = gHR = d3d11_device->CreateDepthStencilView(m_texture.get(), &dsvdef, m_view.put());
        if (FAILED(hr)) {
            Logger::error("Windows API failed: ID3D11Device::CreateDepthStencilView");
            return false;
        }

        return true;
    }
}

#include "d3d11/GraphicsDevice.hpp"

namespace core {
    bool GraphicsDevice::createDepthStencilBuffer(const Vector2U size, IDepthStencilBuffer** const out_depth_stencil_buffer) {
        if (out_depth_stencil_buffer == nullptr) {
            assert(false); return false;
        }
        SmartReference<DepthStencilBuffer> buffer;
        buffer.attach(new DepthStencilBuffer);
        if (!buffer->initialize(this, size)) {
            return false;
        }
        *out_depth_stencil_buffer = buffer.detach();
        return true;
    }
}
