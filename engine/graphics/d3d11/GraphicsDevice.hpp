#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    class GraphicsDevice final : public implement::ReferenceCounted<IGraphicsDevice, IGraphicsCommandBuffer> {
    public:
        // IGraphicsDevice

        void* getNativeDevice() override;

        void addEventListener(IGraphicsDeviceEventListener* listener) override;
        void removeEventListener(IGraphicsDeviceEventListener* listener) override;

        GraphicsDeviceMemoryStatistics getMemoryStatistics() override;

        bool createVertexBuffer(uint32_t size_in_bytes, uint32_t stride_in_bytes, IGraphicsBuffer** output_buffer) override;
        bool createIndexBuffer(uint32_t size_in_bytes, GraphicsFormat format, IGraphicsBuffer** output_buffer) override;
        bool createConstantBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) override;

        bool createTextureFromFile(StringView path, bool mipmap, ITexture2D** out_texture) override;
        bool createTexture(Vector2U size, ITexture2D** out_texture) override;
        bool createTextureFromImage(IImage* image, bool mipmap, ITexture2D** out_texture) override;
        bool createVideoTexture(StringView path, ITexture2D** out_texture) override;
        bool createVideoTexture(StringView path, VideoOpenOptions const& options, ITexture2D** out_texture) override;
        bool createVideoDecoder(IVideoDecoder** out_decoder) override;

        bool createSampler(const GraphicsSamplerInfo& info, IGraphicsSampler** out_sampler) override;

        bool createRenderTarget(Vector2U size, IRenderTarget** out_render_target) override;
        bool createDepthStencilBuffer(Vector2U size, IDepthStencilBuffer** out_depth_stencil_buffer) override;

        bool createGraphicsPipeline(const GraphicsPipelineState* graphics_pipeline_state, IGraphicsPipeline** out_graphics_pipeline) override;

        IGraphicsCommandBuffer* getCommandbuffer() const noexcept override { return const_cast<IGraphicsCommandBuffer*>(static_cast<IGraphicsCommandBuffer const*>(this)); }

        // IGraphicsCommandBuffer

        void* getNativeHandle() const noexcept override;

        void bindVertexBuffer(uint32_t start_slot, IGraphicsBuffer* const* buffers, uint32_t count, uint32_t const* offset) override;
        void bindIndexBuffer(IGraphicsBuffer* buffer, uint32_t offset) override;

        void bindVertexShaderConstantBuffer(uint32_t start_slot, IGraphicsBuffer* const* buffers, uint32_t count) override;
        void bindVertexShaderTexture2D(uint32_t start_slot, ITexture2D* const* textures, uint32_t count) override;
        void bindVertexShaderSampler(uint32_t start_slot, IGraphicsSampler* const* samplers, uint32_t count) override;

        void setViewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
        void setScissorRect(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        void bindPixelShaderConstantBuffer(uint32_t start_slot, IGraphicsBuffer* const* buffers, uint32_t count) override;
        void bindPixelShaderTexture2D(uint32_t start_slot, ITexture2D* const* textures, uint32_t count) override;
        void bindPixelShaderSampler(uint32_t start_slot, IGraphicsSampler* const* samplers, uint32_t count) override;

        void bindRenderTarget(IRenderTarget* render_target, IDepthStencilBuffer* depth_stencil_buffer) override;

        void bindGraphicsPipeline(IGraphicsPipeline* graphics_pipeline) override;

        void draw(uint32_t vertex_count, uint32_t first_vertex) override;
        void drawInstanced(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
        void drawIndexed(uint32_t index_count, uint32_t first_index, int32_t vertex_offset) override;
        void drawIndexedInstanced(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override;

        // from IDevice

        bool recreate() override;
        void setPreferenceGpu(StringView preferred_gpu) override { preferred_adapter_name = preferred_gpu; }
        uint32_t getGpuCount() override { return static_cast<uint32_t>(dxgi_adapter_name_list.size()); }
        StringView getGpuName(uint32_t index) override { return dxgi_adapter_name_list[index]; }
        StringView getCurrentGpuName() const noexcept override { return dxgi_adapter_name; }

#ifdef LUASTG_ENABLE_DIRECT2D
        void* getNativeRendererHandle() override { return d2d1_devctx.get(); }
#else
        void* getNativeRendererHandle() override { return nullptr; }
#endif

        // GraphicsDevice

        GraphicsDevice();
        GraphicsDevice(GraphicsDevice const&) = delete;
        GraphicsDevice(GraphicsDevice&&) = delete;
        ~GraphicsDevice();

        GraphicsDevice& operator=(GraphicsDevice const&) = delete;
        GraphicsDevice& operator=(GraphicsDevice&&) = delete;

        ID3D11Device* GetD3D11Device() const noexcept { return d3d11_device.get(); }
        ID3D11DeviceContext* GetD3D11DeviceContext() const noexcept { return d3d11_devctx.get(); }
        ID3D11DeviceContext1* GetD3D11DeviceContext1() const noexcept { return d3d11_devctx1.get(); }

#ifdef LUASTG_ENABLE_DIRECT2D
        ID2D1Device* GetD2D1Device() const noexcept { return d2d1_device.get(); }
        ID2D1DeviceContext* GetD2D1DeviceContext() const noexcept { return d2d1_devctx.get(); }
#endif

        tracy_d3d11_context_t GetTracyContext() const noexcept { return tracy_context; }

        bool create();
        void destroy();
        bool handleDeviceLost();

        void removeGraphicsPipelineCache(core::IGraphicsPipeline* graphics_pipeline);

    private:
        enum class Event {
            create,
            destroy,
        };

        void dispatchEvent(Event e);

        bool testAdapterPolicy();
        bool selectAdapter();
        bool createDXGI();
        void destroyDXGI();
        bool createD3D11();
        void destroyD3D11();
#ifdef LUASTG_ENABLE_DIRECT2D
        bool createD2D1();
        void destroyD2D1();
#endif
        bool doDestroyAndCreate();

        // Event dispatcher

        std::vector<IGraphicsDeviceEventListener*> m_event_listeners;
        bool m_is_dispatching_event{ false };

        // DXGI

        win32::com_ptr<IDXGIFactory2> dxgi_factory;
        win32::com_ptr<IDXGIAdapter1> dxgi_adapter;

        std::string preferred_adapter_name;

        std::string dxgi_adapter_name;
        std::vector<std::string> dxgi_adapter_name_list;

        // Direct3D

        D3D_FEATURE_LEVEL d3d_feature_level{ D3D_FEATURE_LEVEL_10_0 };

        // Direct3D 11

        win32::com_ptr<ID3D11Device> d3d11_device;
        win32::com_ptr<ID3D11DeviceContext> d3d11_devctx;
        win32::com_ptr<ID3D11DeviceContext1> d3d11_devctx1;
        
        // Graphics

        std::unordered_set<IGraphicsPipeline*> m_graphics_pipeline_cache;

        // Direct2D 1

#ifdef LUASTG_ENABLE_DIRECT2D
        win32::com_ptr<ID2D1Factory1> d2d1_factory;
        win32::com_ptr<ID2D1Device> d2d1_device;
        win32::com_ptr<ID2D1DeviceContext> d2d1_devctx;
#endif

        // Debug

        tracy_d3d11_context_t tracy_context{};
    };
}
