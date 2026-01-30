#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    class GraphicsDevice final : public implement::ReferenceCounted<IGraphicsDevice> {
    public:
        // IGraphicsDevice

        void* getNativeDevice() override;

        void addEventListener(IGraphicsDeviceEventListener* listener) override;
        void removeEventListener(IGraphicsDeviceEventListener* listener) override;

        bool createVertexBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) override;
        bool createIndexBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) override;
        bool createConstantBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) override;

        // from IDevice

        GraphicsDeviceMemoryStatistics getMemoryStatistics() override;

        bool recreate() override;
        void setPreferenceGpu(StringView preferred_gpu) override { preferred_adapter_name = preferred_gpu; }
        uint32_t getGpuCount() override { return static_cast<uint32_t>(dxgi_adapter_name_list.size()); }
        StringView getGpuName(uint32_t index) override { return dxgi_adapter_name_list[index]; }
        StringView getCurrentGpuName() const noexcept override { return dxgi_adapter_name; }

        void* getNativeHandle() override { return d3d11_device.get(); }
#ifdef LUASTG_ENABLE_DIRECT2D
        void* getNativeRendererHandle() override { return d2d1_devctx.get(); }
#else
        void* getNativeRendererHandle() override { return nullptr; }
#endif

        bool createTextureFromFile(StringView path, bool mipmap, Graphics::ITexture2D** pp_texture) override;
        bool createTexture(Vector2U size, Graphics::ITexture2D** pp_texture) override;
        bool createTextureFromImage(IImage* image, bool mipmap, Graphics::ITexture2D** pp_texture) override;
        bool createRenderTarget(Vector2U size, Graphics::IRenderTarget** pp_rt) override;

        bool createDepthStencilBuffer(Vector2U size, Graphics::IDepthStencilBuffer** pp_ds) override;

        bool createSamplerState(core::Graphics::SamplerState const& info, Graphics::ISamplerState** pp_sampler) override;

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

        IWICImagingFactory* GetWICImagingFactory() const noexcept { return wic_factory.get(); }

        tracy_d3d11_context_t GetTracyContext() const noexcept { return tracy_context; }

        bool create();
        void destroy();
        bool handleDeviceLost();

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
        bool createWIC();
        void destroyWIC();
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

        // Window Image Component

        win32::com_ptr<IWICImagingFactory> wic_factory;
        win32::com_ptr<IWICImagingFactory2> wic_factory2;

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
