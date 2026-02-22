#include "d3d11/VideoTexture.hpp"
#include "d3d11/VideoDecoder.hpp"
#include "d3d11/GraphicsDevice.hpp"
#include "core/Logger.hpp"
#include "core/SmartReference.hpp"

namespace core {
    VideoTexture::VideoTexture() = default;
    
    VideoTexture::~VideoTexture() {
        if (m_initialized && static_cast<bool>(m_device)) {
            m_device->removeEventListener(this);
        }
    }
    
    bool VideoTexture::initialize(IGraphicsDevice* device, StringView path) {
        return initialize(device, path, VideoOpenOptions{});
    }

    bool VideoTexture::initialize(IGraphicsDevice* device, StringView path, VideoOpenOptions const& options) {
        if (!device) {
            Logger::error("[core] [VideoTexture] Invalid device");
            return false;
        }
        
        m_device = device;
        
        auto decoder = new VideoDecoder();
        if (!decoder->initialize(device)) {
            Logger::error("[core] [VideoTexture] Failed to initialize video decoder");
            decoder->release();
            return false;
        }
        
        if (!decoder->open(path, options)) {
            Logger::error("[core] [VideoTexture] Failed to open video file: {}", path);
            decoder->release();
            return false;
        }
        
        m_decoder = decoder;
        decoder->release();
        m_premultiplied_alpha = options.premultiplied_alpha;
        m_initialized = true;
        m_device->addEventListener(this);

        Logger::info("[core] [VideoTexture] Created video texture from: {}", path);
        return true;
    }

    
    void* VideoTexture::getNativeResource() const noexcept {
        if (m_decoder) {
            return m_decoder->getNativeTexture(); // ID3D11Texture2D*
        }
        return nullptr;
    }
    
    void* VideoTexture::getNativeView() const noexcept {
        if (m_decoder) {
            return m_decoder->getNativeShaderResourceView(); // ID3D11ShaderResourceView*
        }
        return nullptr;
    }
    
    Vector2U VideoTexture::getSize() const noexcept {
        if (m_decoder) {
            return m_decoder->getVideoSize();
        }
        return Vector2U{};
    }
    
    bool VideoTexture::setSize(Vector2U /* size */) {
        Logger::warn("[core] [VideoTexture] setSize() is not supported on video textures");
        return false;
    }
    
    bool VideoTexture::update(RectU /* rect */, void const* /* data */, uint32_t /* row_pitch_in_bytes */) {
        Logger::warn("[core] [VideoTexture] update() is not supported on video textures");
        return false;
    }
    
    void VideoTexture::setImage(IImage* /* image */) {
        Logger::warn("[core] [VideoTexture] setImage() is not supported on video textures");
    }
    
    bool VideoTexture::saveToFile(StringView /* path */) {
        Logger::warn("[core] [VideoTexture] saveToFile() is not supported on video textures");
        return false;
    }
    
    void VideoTexture::onGraphicsDeviceCreate() {}
    
    void VideoTexture::onGraphicsDeviceDestroy() {}
    
    // GraphicsDevice 扩展：视频功能
    
    bool GraphicsDevice::createVideoTexture(StringView path, ITexture2D** out_texture) {
        return createVideoTexture(path, VideoOpenOptions{}, out_texture);
    }

    bool GraphicsDevice::createVideoTexture(StringView path, VideoOpenOptions const& options, ITexture2D** out_texture) {
        if (out_texture == nullptr) {
            assert(false); return false;
        }
        SmartReference<VideoTexture> video_texture;
        video_texture.attach(new VideoTexture);
        if (!video_texture->initialize(this, path, options)) {
            return false;
        }
        *out_texture = video_texture.detach();
        return true;
    }
    
    bool GraphicsDevice::createVideoDecoder(IVideoDecoder** out_decoder) {
        if (out_decoder == nullptr) {
            assert(false); return false;
        }
        SmartReference<VideoDecoder> decoder;
        decoder.attach(new VideoDecoder);
        if (!decoder->initialize(this)) {
            return false;
        }
        *out_decoder = decoder.detach();
        return true;
    }
}
