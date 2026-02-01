#include "d3d11/GraphicsSampler.hpp"
#include "core/Logger.hpp"

namespace {
    D3D11_FILTER toFilter(const core::GraphicsFilter filter) {
    #define FILTER_VALUE(MIN, MAG, MIP) core::GraphicsFilter::min_##MIN##_mag_##MAG##_mip_##MIP
        switch (filter) {
        case FILTER_VALUE(point , point , point ): return D3D11_FILTER_MIN_MAG_MIP_POINT; break;
        case FILTER_VALUE(linear, point , point ): return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; break;
        case FILTER_VALUE(point , linear, point ): return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; break;
        case FILTER_VALUE(point , point , linear): return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; break;
        case FILTER_VALUE(linear, linear, point ): return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; break;
        case FILTER_VALUE(linear, point , linear): return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; break;
        case FILTER_VALUE(point , linear, linear): return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR; break;
        case FILTER_VALUE(linear, linear, linear): return D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
        case core::GraphicsFilter::anisotropic: return D3D11_FILTER_ANISOTROPIC; break;
        default: assert(false); return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        }
    #undef FILTER_VALUE
    }
    D3D11_TEXTURE_ADDRESS_MODE toTextureAddressMode(const core::GraphicsTextureAddressMode mode) {
        switch (mode) {
        case core::GraphicsTextureAddressMode::wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
        case core::GraphicsTextureAddressMode::clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
        default: assert(false); return D3D11_TEXTURE_ADDRESS_CLAMP;
        }
    }
}

namespace core {
    // IGraphicsDeviceEventListener

    void GraphicsSampler::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResource();
        }
    }
    void GraphicsSampler::onGraphicsDeviceDestroy() {
        m_sampler.reset();
    }

    // GraphicsSampler

    GraphicsSampler::GraphicsSampler() = default;
    GraphicsSampler::~GraphicsSampler() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool GraphicsSampler::initialize(IGraphicsDevice* const device, const GraphicsSamplerInfo& info) {
        assert(device);
        m_device = device;
        m_info = info;
        if (!createResource()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
    bool GraphicsSampler::createResource() {
        const auto device = static_cast<ID3D11Device*>(m_device->getNativeHandle());
        if (device == nullptr) {
            assert(false); return false;
        }
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = toFilter(m_info.filter);
        desc.AddressU = toTextureAddressMode(m_info.address_u);
        desc.AddressV = toTextureAddressMode(m_info.address_v);
        desc.AddressW = toTextureAddressMode(m_info.address_w);
        desc.MipLODBias = m_info.mip_lod_bias;
        desc.MaxAnisotropy = m_info.max_anisotropy;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = m_info.min_lod;
        desc.MaxLOD = m_info.max_lod;
        return win32::check_hresult_as_boolean(
            device->CreateSamplerState(&desc, m_sampler.put()),
            "ID3D11Device::CreateSamplerState"
        );
    }
}

#include "d3d11/GraphicsDevice.hpp"

namespace core {
    bool GraphicsDevice::createSampler(const GraphicsSamplerInfo& info, IGraphicsSampler** const out_sampler) {
        if (out_sampler == nullptr) {
            assert(false); return false;
        }
        SmartReference<GraphicsSampler> buffer;
        buffer.attach(new GraphicsSampler());
        if (!buffer->initialize(this, info)) {
            return false;
        }
        *out_sampler = buffer.detach();
        return true;
    }
}
