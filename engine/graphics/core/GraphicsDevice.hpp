#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/ImmutableString.hpp"
#include "core/GraphicsBuffer.hpp"

#include "core/Graphics/Device.hpp"

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

    CORE_INTERFACE IGraphicsDevice : IReferenceCounted {
        virtual void* getNativeDevice() = 0;

        virtual void addEventListener(IGraphicsDeviceEventListener* listener) = 0;
        virtual void removeEventListener(IGraphicsDeviceEventListener* listener) = 0;

        virtual bool createVertexBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) = 0;
        virtual bool createIndexBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) = 0;
        virtual bool createConstantBuffer(uint32_t size_in_bytes, IGraphicsBuffer** output_buffer) = 0;

        static bool create(StringView preferred_gpu, IGraphicsDevice** output);

        // from IDevice

        virtual GraphicsDeviceMemoryStatistics getMemoryStatistics() = 0;

        virtual bool recreate() = 0;
        virtual void setPreferenceGpu(StringView preferred_gpu) = 0;
        virtual uint32_t getGpuCount() = 0;
        virtual StringView getGpuName(uint32_t index) = 0;
        virtual StringView getCurrentGpuName() const noexcept = 0;

        virtual void* getNativeHandle() = 0;
        virtual void* getNativeRendererHandle() = 0;

        virtual bool createTextureFromFile(StringView path, bool mipmap, Graphics::ITexture2D** pp_texture) = 0;
        virtual bool createTextureFromImage(IImage* image, bool mipmap, Graphics::ITexture2D** pp_texture) = 0;
        virtual bool createTexture(Vector2U size, Graphics::ITexture2D** pp_texture) = 0;

        virtual bool createRenderTarget(Vector2U size, Graphics::IRenderTarget** pp_rt) = 0;
        virtual bool createDepthStencilBuffer(Vector2U size, Graphics::IDepthStencilBuffer** pp_ds) = 0;

        virtual bool createSamplerState(Graphics::SamplerState const& info, Graphics::ISamplerState** pp_sampler) = 0;
    };

    CORE_INTERFACE_ID(IGraphicsDevice, "17b76b63-ceb6-5f87-aa5f-366e89d7176e")
}
