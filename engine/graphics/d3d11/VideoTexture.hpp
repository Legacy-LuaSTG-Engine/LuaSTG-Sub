#pragma once
#include "core/Texture2D.hpp"
#include "core/VideoDecoder.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"

namespace core {
    // VideoTexture 类：结合了 Texture2D 和 VideoDecoder 的功能
    // 可以像普通纹理一样使用，但内容来自视频
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
        
        bool setSize(Vector2U /* size */) override;
        bool update(RectU /* rect */, void const* /* data */, uint32_t /* row_pitch_in_bytes */) override;
        void setImage(IImage* /* image */) override;
        
        bool saveToFile(StringView /* path */) override;
        
        void setSamplerState(IGraphicsSampler* sampler) override { m_sampler = sampler; }
        IGraphicsSampler* getSamplerState() const noexcept override { return m_sampler.get(); }
        
        IVideoDecoder* getVideoDecoder() const noexcept override { return m_decoder.get(); }
        
        // IGraphicsDeviceEventListener
        
        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;
        
        // VideoTexture
        
        VideoTexture();
        ~VideoTexture();
        
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
