#include "d3d11/GraphicsDevice.hpp"

namespace {
    ID3D11Buffer* getBuffer(core::IGraphicsBuffer* const buffer) {
        if (buffer != nullptr) {
            return static_cast<ID3D11Buffer*>(buffer->getNativeResource());
        }
        return nullptr;
    }
    UINT getBufferStride(core::IGraphicsBuffer* const buffer) {
        if (buffer != nullptr) {
            return buffer->getStrideInBytes();
        }
        return 0u;
    }
    ID3D11ShaderResourceView* getTexture2D(core::ITexture2D* const texture) {
        if (texture != nullptr) {
            return static_cast<ID3D11ShaderResourceView*>(texture->getNativeView());
        }
        return nullptr;
    }
    ID3D11SamplerState* getSampler(core::IGraphicsSampler* const sampler) {
        if (sampler != nullptr) {
            return static_cast<ID3D11SamplerState*>(sampler->getNativeHandle());
        }
        return nullptr;
    }
    ID3D11RenderTargetView* getRenderTarget(core::IRenderTarget* const render_target) {
        if (render_target != nullptr) {
            return static_cast<ID3D11RenderTargetView*>(render_target->getNativeView());
        }
        return nullptr;
    }
    ID3D11DepthStencilView* getDepthStencilBuffer(core::IDepthStencilBuffer* const depth_stencil_buffer) {
        if (depth_stencil_buffer != nullptr) {
            return static_cast<ID3D11DepthStencilView*>(depth_stencil_buffer->getNativeView());
        }
        return nullptr;
    }
}

namespace core {
    // NOTE: 由于我们设定的基线是 feature level 10.0 ，下方代码引用的数量限制常量均为 D3D10 的

    void* GraphicsDevice::getNativeHandle() const noexcept {
        return d3d11_devctx.get();
    }

    void GraphicsDevice::bindVertexBuffer(const uint32_t start_slot, IGraphicsBuffer* const* const buffers, const uint32_t count, uint32_t const* const offset) {
        assert(start_slot < D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT);
        assert(buffers != nullptr);
        assert(count > 0 && count <= (D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - start_slot));
        assert(offset != nullptr);
        ID3D11Buffer* b[D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]{};
        UINT s[D3D10_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            b[i] = getBuffer(buffers[i]);
            s[i] = getBufferStride(buffers[i]);
        }
        d3d11_devctx->IASetVertexBuffers(start_slot, count, b, s, offset);
    }
    void GraphicsDevice::bindIndexBuffer(IGraphicsBuffer* const buffer, const uint32_t offset) {
        const auto format = getBufferStride(buffer) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        d3d11_devctx->IASetIndexBuffer(getBuffer(buffer), format, offset);
    }

    void GraphicsDevice::bindVertexShaderConstantBuffer(const uint32_t start_slot, IGraphicsBuffer* const* const buffers, const uint32_t count) {
        assert(start_slot < D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
        assert(buffers != nullptr);
        assert(count > 0 && count <= (D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - start_slot));
        ID3D11Buffer* s[D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            s[i] = getBuffer(buffers[i]);
        }
        d3d11_devctx->VSSetConstantBuffers(start_slot, count, s);
    }
    void GraphicsDevice::bindVertexShaderTexture2D(const uint32_t start_slot, ITexture2D* const* const textures, const uint32_t count) {
        assert(start_slot < D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
        assert(textures != nullptr);
        assert(count > 0 && count <= (D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - start_slot));
        ID3D11ShaderResourceView* s[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            s[i] = getTexture2D(textures[i]);
        }
        d3d11_devctx->VSSetShaderResources(start_slot, count, s);
    }
    void GraphicsDevice::bindVertexShaderSampler(const uint32_t start_slot, IGraphicsSampler* const* const samplers, const uint32_t count) {
        assert(start_slot < D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT);
        assert(samplers != nullptr);
        assert(count > 0 && count <= (D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT - start_slot));
        ID3D11SamplerState* s[D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            s[i] = getSampler(samplers[i]);
        }
        d3d11_devctx->VSSetSamplers(start_slot, count, s);
    }

    void GraphicsDevice::setViewport(const float x, const float y, const float width, const float height, const float min_depth, const float max_depth) {
        const D3D11_VIEWPORT viewport{
            .TopLeftX = x,
            .TopLeftY = y,
            .Width = width,
            .Height = height,
            .MinDepth = min_depth,
            .MaxDepth = max_depth,
        };
        d3d11_devctx->RSSetViewports(1, &viewport);
    }
    void GraphicsDevice::setScissorRect(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) {
        const D3D11_RECT rect{
            .left = x,
            .top = y,
            .right = static_cast<LONG>(x + width),
            .bottom = static_cast<LONG>(y + height)
        };
        d3d11_devctx->RSSetScissorRects(1, &rect);
    }

    void GraphicsDevice::bindPixelShaderConstantBuffer(const uint32_t start_slot, IGraphicsBuffer* const* const buffers, const uint32_t count) {
        assert(start_slot < D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
        assert(buffers != nullptr);
        assert(count > 0 && count <= (D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - start_slot));
        ID3D11Buffer* s[D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            s[i] = getBuffer(buffers[i]);
        }
        d3d11_devctx->PSSetConstantBuffers(start_slot, count, s);
    }
    void GraphicsDevice::bindPixelShaderTexture2D(const uint32_t start_slot, ITexture2D* const* const textures, const uint32_t count) {
        assert(start_slot < D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
        assert(textures != nullptr);
        assert(count > 0 && count <= (D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - start_slot));
        ID3D11ShaderResourceView* s[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            s[i] = getTexture2D(textures[i]);
        }
        d3d11_devctx->PSSetShaderResources(start_slot, count, s);
    }
    void GraphicsDevice::bindPixelShaderSampler(const uint32_t start_slot, IGraphicsSampler* const* const samplers, const uint32_t count) {
        assert(start_slot < D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT);
        assert(samplers != nullptr);
        assert(count > 0 && count <= (D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT - start_slot));
        ID3D11SamplerState* s[D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT]{};
        for (uint32_t i = 0; i < count; i += 1) {
            s[i] = getSampler(samplers[i]);
        }
        d3d11_devctx->PSSetSamplers(start_slot, count, s);
    }

    void GraphicsDevice::bindRenderTarget(IRenderTarget* const render_target, IDepthStencilBuffer* const depth_stencil_buffer) {
        assert(render_target != nullptr);
        const auto rtv = getRenderTarget(render_target);
        d3d11_devctx->OMSetRenderTargets(1, &rtv, getDepthStencilBuffer(depth_stencil_buffer));
    }
}
