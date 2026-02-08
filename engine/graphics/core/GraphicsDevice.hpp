#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/ImmutableString.hpp"
#include "core/GraphicsBuffer.hpp"
#include "core/Texture2D.hpp"
#include "core/GraphicsSampler.hpp"
#include "core/RenderTarget.hpp"
#include "core/DepthStencilBuffer.hpp"
#include "core/GraphicsPipeline.hpp"

#define LUASTG_ENABLE_DIRECT2D

namespace core {
    CORE_INTERFACE IGraphicsDeviceEventListener {
        virtual void onGraphicsDeviceCreate() = 0;
        virtual void onGraphicsDeviceDestroy() = 0;
    };

    struct GraphicsDeviceMemoryStatistics {
        struct Scope {
            uint64_t budget;
            uint64_t current_usage;
            uint64_t available_for_reservation;
            uint64_t current_reservation;
        };
        Scope local{};
        Scope non_local{};
    };

    CORE_INTERFACE IGraphicsCommandBuffer : IReferenceCounted {
        virtual void* getNativeHandle() const noexcept = 0;

        virtual void bindVertexBuffer(uint32_t start_slot, IGraphicsBuffer* const* buffers, uint32_t count, uint32_t const* offset) = 0;
        virtual void bindIndexBuffer(IGraphicsBuffer* buffer, uint32_t offset) = 0;

        virtual void bindVertexShaderConstantBuffer(uint32_t start_slot, IGraphicsBuffer* const* buffers, uint32_t count) = 0;
        virtual void bindVertexShaderTexture2D(uint32_t start_slot, ITexture2D* const* textures, uint32_t count) = 0;
        virtual void bindVertexShaderSampler(uint32_t start_slot, IGraphicsSampler* const* samplers, uint32_t count) = 0;

        virtual void setViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
        virtual void setScissorRect(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        virtual void bindPixelShaderConstantBuffer(uint32_t start_slot, IGraphicsBuffer* const* buffers, uint32_t count) = 0;
        virtual void bindPixelShaderTexture2D(uint32_t start_slot, ITexture2D* const* textures, uint32_t count) = 0;
        virtual void bindPixelShaderSampler(uint32_t start_slot, IGraphicsSampler* const* samplers, uint32_t count) = 0;

        virtual void bindRenderTarget(IRenderTarget* render_target, IDepthStencilBuffer* depth_stencil_buffer) = 0;

        virtual void bindGraphicsPipeline(IGraphicsPipeline* graphics_pipeline) = 0;

        // helpers

        void bindVertexBuffer(const uint32_t slot, IGraphicsBuffer* const buffer, const uint32_t offset = 0) {
            bindVertexBuffer(slot, &buffer, 1, &offset);
        }
        void bindIndexBuffer(IGraphicsBuffer* const buffer) {
            bindIndexBuffer(buffer, 0);
        }

        void bindVertexShaderConstantBuffer(const uint32_t slot, IGraphicsBuffer* const buffer) {
            return bindVertexShaderConstantBuffer(slot, &buffer, 1);
        }
        void bindVertexShaderTexture2D(const uint32_t slot, ITexture2D* const texture) {
            return bindVertexShaderTexture2D(slot, &texture, 1);
        }
        void bindVertexShaderSampler(const uint32_t slot, IGraphicsSampler* const sampler) {
            bindVertexShaderSampler(slot, &sampler, 1);
        }

        void setViewport(const float x, const float y, const float width, const float height) {
            setViewport(x, y, width, height, 0.0f, 1.0f);
        }

        void bindPixelShaderConstantBuffer(const uint32_t slot, IGraphicsBuffer* const buffer) {
            bindPixelShaderConstantBuffer(slot, &buffer, 1);
        }
        void bindPixelShaderTexture2D(const uint32_t slot, ITexture2D* const texture) {
            return bindPixelShaderTexture2D(slot, &texture, 1);
        }
        void bindPixelShaderSampler(const uint32_t slot, IGraphicsSampler* const sampler) {
            bindPixelShaderSampler(slot, &sampler, 1);
        }

        void bindRenderTarget(IRenderTarget* const render_target) {
            return bindRenderTarget(render_target, nullptr);
        }
    };

    CORE_INTERFACE_ID(IGraphicsCommandBuffer, "ad182f92-e387-5055-ae9b-81d69663f304");

    CORE_INTERFACE IGraphicsDevice : IReferenceCounted {
        virtual void* getNativeDevice() = 0;

        virtual void addEventListener(IGraphicsDeviceEventListener* listener) = 0;
        virtual void removeEventListener(IGraphicsDeviceEventListener* listener) = 0;

        virtual GraphicsDeviceMemoryStatistics getMemoryStatistics() = 0;

        virtual bool createVertexBuffer(uint32_t size_in_bytes, uint32_t stride_in_bytes, IGraphicsBuffer** output_buffer) = 0;
        virtual bool createIndexBuffer(uint32_t size_in_bytes, GraphicsFormat format, IGraphicsBuffer** output_buffer) = 0;
        virtual bool createConstantBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) = 0;

        virtual bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** out_texture) = 0;
        virtual bool createTextureFromImage(IImage* image, bool mipmap, ITexture2D** out_texture) = 0;
        virtual bool createTexture(Vector2U size, ITexture2D** out_texture) = 0;

        virtual bool createSampler(const GraphicsSamplerInfo& info, IGraphicsSampler** out_sampler) = 0;

        virtual bool createRenderTarget(Vector2U size, IRenderTarget** out_render_target) = 0;
        virtual bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** out_depth_stencil_buffer) = 0;

        virtual bool createGraphicsPipeline(const GraphicsPipelineState* graphics_pipeline_state, IGraphicsPipeline** out_graphics_pipeline) = 0;

        virtual IGraphicsCommandBuffer* getCommandbuffer() const noexcept = 0;

        static bool create(StringView preferred_gpu, IGraphicsDevice** output);

        // from IDevice

        virtual bool recreate() = 0;
        virtual void setPreferenceGpu(StringView preferred_gpu) = 0;
        virtual uint32_t getGpuCount() = 0;
        virtual StringView getGpuName(uint32_t index) = 0;
        virtual StringView getCurrentGpuName() const noexcept = 0;

        virtual void* getNativeHandle() = 0;
        virtual void* getNativeRendererHandle() = 0;
    };

    CORE_INTERFACE_ID(IGraphicsDevice, "17b76b63-ceb6-5f87-aa5f-366e89d7176e");
}
