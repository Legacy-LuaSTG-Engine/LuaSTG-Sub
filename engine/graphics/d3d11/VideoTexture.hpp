#pragma once
#include "core/Texture2D.hpp"
#include "core/VideoDecoder.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    class VideoTexture final :
        public implement::ReferenceCounted<ITexture2D>,
        public IGraphicsDeviceEventListener {
    public:
        // ITexture2D

        void* getNativeResource() const noexcept override;
        void* getNativeView() const noexcept override;

        bool isDynamic() const noexcept override { return true; }
        bool isVideoTexture() const noexcept override { return true; }
        bool isPremultipliedAlpha() const noexcept override { return m_premultiplied_alpha; }
        void setPremultipliedAlpha(bool v) override { m_premultiplied_alpha = v; }
        Vector2U getSize() const noexcept override;

        bool setSize(Vector2U) override;
        bool update(RectU, void const*, uint32_t) override;
        void setImage(IImage*) override;

        bool saveToFile(StringView) override;

        void setSamplerState(IGraphicsSampler* sampler) override { m_sampler = sampler; }
        IGraphicsSampler* getSamplerState() const noexcept override { return m_sampler.get(); }

        IVideoDecoder* getVideoDecoder() const noexcept override { return m_decoder.get(); }

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // VideoTexture

        VideoTexture();
        VideoTexture(VideoTexture const&) = delete;
        VideoTexture(VideoTexture&&) = delete;
        ~VideoTexture();

        VideoTexture& operator=(VideoTexture const&) = delete;
        VideoTexture& operator=(VideoTexture&&) = delete;

        bool initialize(IGraphicsDevice* device, StringView path);
        bool initialize(IGraphicsDevice* device, StringView path, VideoOpenOptions const& options);

    private:
        SmartReference<IGraphicsDevice> m_device;
        SmartReference<IVideoDecoder> m_decoder;
        SmartReference<IGraphicsSampler> m_sampler;
        bool m_premultiplied_alpha{ false };
        bool m_initialized{ false };
    };
}
