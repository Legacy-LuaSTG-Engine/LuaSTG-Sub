#include "d3d11/VideoDecoder.hpp"
#include "core/Logger.hpp"
#include "utf8.hpp"
#include <mferror.h>
#include <mfobjects.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace {
    class MFInitializer {
    public:
        static MFInitializer& getInstance() {
            static MFInitializer instance;
            return instance;
        }
        
        bool isInitialized() const { return m_initialized; }
        
    private:
        MFInitializer() {
            HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
            m_initialized = SUCCEEDED(hr);
            if (!m_initialized) {
                core::Logger::error("[core] [VideoDecoder] Failed to initialize MediaFoundation, hr = {:#x}", (uint32_t)hr);
            }
        }
        
        ~MFInitializer() {
            if (m_initialized) {
                MFShutdown();
            }
        }
        
        bool m_initialized{ false };
    };
}

namespace core {
    VideoDecoder::VideoDecoder() = default;
    
    VideoDecoder::~VideoDecoder() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
        close();
    }
    
    bool VideoDecoder::initialize(IGraphicsDevice* device) {
        if (!device) {
            Logger::error("[core] [VideoDecoder] Invalid device");
            return false;
        }
        
        if (!MFInitializer::getInstance().isInitialized()) {
            Logger::error("[core] [VideoDecoder] MediaFoundation not initialized");
            return false;
        }
        
        m_device = device;
        m_initialized = true;
        m_device->addEventListener(this);
        
        return true;
    }
    
    bool VideoDecoder::open(StringView path) {
        if (!m_initialized) {
            Logger::error("[core] [VideoDecoder] Not initialized");
            return false;
        }
        
        close();
        
        HRESULT hr = S_OK;
        
        std::wstring wide_path = utf8::to_wstring(path);
        
        win32::com_ptr<IMFAttributes> attributes;
        hr = MFCreateAttributes(attributes.put(), 1);
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create attributes, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        hr = attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
        hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
        
        hr = MFCreateSourceReaderFromURL(wide_path.c_str(), attributes.get(), m_source_reader.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create source reader, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        win32::com_ptr<IMFMediaType> media_type;
        hr = MFCreateMediaType(media_type.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create media type, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        
        hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
        hr = m_source_reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, media_type.get());
        
        if (FAILED(hr)) {
            Logger::info("[core] [VideoDecoder] ARGB32 not supported, trying RGB32");
            hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
            hr = m_source_reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, media_type.get());
            
            if (FAILED(hr)) {
                Logger::error("[core] [VideoDecoder] Failed to set media type (tried ARGB32 and RGB32), hr = {:#x}", (uint32_t)hr);
                return false;
            }
        }
        
        hr = m_source_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, m_media_type.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to get media type, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        GUID subtype = GUID_NULL;
        hr = m_media_type->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr)) {
            if (subtype == MFVideoFormat_ARGB32) {
                Logger::info("[core] [VideoDecoder] Using video format: ARGB32 (native BGRA)");
            } else if (subtype == MFVideoFormat_RGB32) {
                Logger::info("[core] [VideoDecoder] Using video format: RGB32");
            } else {
                Logger::warn("[core] [VideoDecoder] Using unknown video format, display may be incorrect");
            }
        }
        
        UINT32 width = 0, height = 0;
        hr = MFGetAttributeSize(m_media_type.get(), MF_MT_FRAME_SIZE, &width, &height);
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to get frame size, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        m_video_size = Vector2U{ width, height };
        m_target_size = m_video_size;
        
        PROPVARIANT var;
        PropVariantInit(&var);
        hr = m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
        if (SUCCEEDED(hr)) {
            m_duration = var.hVal.QuadPart / 10000000.0;
            PropVariantClear(&var);
        }
        
        if (!createTexture()) {
            close();
            return false;
        }
        
        m_frame_pitch = m_target_size.x * 4;
        
        auto d3d_device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (d3d_device) {
            d3d_device->GetImmediateContext(m_device_context.put());
        }
        
        m_current_time = 0.0;
        m_last_requested_time = -1.0;
        
        bool first_frame_loaded = readFrameAtTime(0.0);
        Logger::info("[core] [VideoDecoder] First frame loaded: {}", first_frame_loaded);
        
        Logger::info("[core] [VideoDecoder] Opened video: {}x{}, duration: {:.2f}s", 
                    m_target_size.x, m_target_size.y, m_duration);
        
        return true;
    }
    
    void VideoDecoder::close() {
        m_source_reader.reset();
        m_media_type.reset();
        m_texture.reset();
        m_shader_resource_view.reset();
        m_device_context.reset();
        
        m_video_size = Vector2U{};
        m_target_size = Vector2U{};
        m_duration = 0.0;
        m_current_time = 0.0;
        m_last_requested_time = -1.0;
        m_frame_pitch = 0;
    }
    
    bool VideoDecoder::seek(double time_in_seconds) {
        if (!hasVideo()) {
            return false;
        }
        
        if (time_in_seconds < 0.0) {
            time_in_seconds = 0.0;
        }
        if (time_in_seconds > m_duration) {
            time_in_seconds = m_duration;
        }
        
        PROPVARIANT var;
        PropVariantInit(&var);
        var.vt = VT_I8;
        var.hVal.QuadPart = static_cast<LONGLONG>(time_in_seconds * 10000000.0);
        
        HRESULT hr = m_source_reader->SetCurrentPosition(GUID_NULL, var);
        PropVariantClear(&var);
        
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to seek, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        m_current_time = time_in_seconds;
        return true;
    }
    
    bool VideoDecoder::updateToTime(double time_in_seconds) {
        if (!hasVideo()) {
            return false;
        }

        if (m_duration <= 0.0) {
            m_current_time = 0.0;
            return false;
        }
        
        if (time_in_seconds >= m_duration) {
            if (m_looping) {
                time_in_seconds = fmod(time_in_seconds, m_duration);
            } else {
                double const last_frame_time = std::max(0.0, m_duration - 1e-6);
                bool const ok = readFrameAtTime(last_frame_time);
                m_current_time = m_duration;
                return ok;
            }
        } else if (time_in_seconds < 0.0) {
            time_in_seconds = 0.0;
        }
        
        constexpr double kTimeEpsilon = 1e-4;
        constexpr double kBackwardTolerance = 1.0 / 120.0;
        constexpr double kFrameTolerance = 1.0 / 24.0;
        constexpr double kSeekThreshold = 0.25;

        bool const is_backward = (m_last_requested_time >= 0.0) && (time_in_seconds + kBackwardTolerance < m_last_requested_time);
        bool const large_jump = (m_current_time + kSeekThreshold < time_in_seconds);

        m_last_requested_time = time_in_seconds;

        if (is_backward || large_jump) {
            return readFrameAtTime(time_in_seconds);
        }

        if (m_current_time + kFrameTolerance >= time_in_seconds) {
            return true;
        }

        for (int i = 0; i < 4; ++i) {
            if (!readNextFrame()) {
                return false;
            }
            if (m_current_time + kTimeEpsilon >= time_in_seconds) {
                return true;
            }
        }

        return readFrameAtTime(time_in_seconds);
    }

    bool VideoDecoder::readNextFrame() {
        if (!hasVideo()) {
            return false;
        }

        HRESULT hr = S_OK;
        win32::com_ptr<IMFSample> sample;
        DWORD stream_flags = 0;
        LONGLONG timestamp = 0;

        hr = m_source_reader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            nullptr,
            &stream_flags,
            &timestamp,
            sample.put()
        );

        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to read sample, hr = {:#x}", (uint32_t)hr);
            return false;
        }

        if (stream_flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            m_current_time = m_duration;
            return false;
        }

        if (!sample) {
            return false;
        }

        m_current_time = timestamp / 10000000.0;
        if (m_current_time > m_duration) {
            m_current_time = m_duration;
        }

        return updateTextureFromSample(sample.get());
    }
    
    bool VideoDecoder::readFrameAtTime(double time_in_seconds) {
        if (!hasVideo()) {
            return false;
        }

        if (!seek(time_in_seconds)) {
            return false;
        }

        constexpr double kTimeEpsilon = 1e-4;
        constexpr int kMaxDecodeAfterSeek = 360;

        for (int i = 0; i < kMaxDecodeAfterSeek; ++i) {
            if (!readNextFrame()) {
                return false;
            }
            if (m_current_time + kTimeEpsilon >= time_in_seconds) {
                return true;
            }
        }

        return false;
    }
    
    void VideoDecoder::onGraphicsDeviceCreate() {
        if (hasVideo()) {
            createTexture();
        }
    }
    
    void VideoDecoder::onGraphicsDeviceDestroy() {
        m_texture.reset();
        m_shader_resource_view.reset();
        m_device_context.reset();
    }
    
    bool VideoDecoder::createTexture() {
        if (!m_device || m_target_size.x == 0 || m_target_size.y == 0) {
            return false;
        }
        
        auto d3d_device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!d3d_device) {
            return false;
        }
        
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = m_target_size.x;
        desc.Height = m_target_size.y;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        
        HRESULT hr = d3d_device->CreateTexture2D(&desc, nullptr, m_texture.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create texture, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        srv_desc.Texture2D.MostDetailedMip = 0;
        
        hr = d3d_device->CreateShaderResourceView(m_texture.get(), &srv_desc, m_shader_resource_view.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create shader resource view, hr = {:#x}", (uint32_t)hr);
            m_texture.reset();
            return false;
        }
        
        d3d_device->GetImmediateContext(m_device_context.put());
        
        return true;
    }
    
    bool VideoDecoder::updateTextureFromSample(IMFSample* sample) {
        if (!m_texture || !sample || !m_device_context) {
            return false;
        }
        
        win32::com_ptr<IMFMediaBuffer> buffer;
        HRESULT hr = sample->ConvertToContiguousBuffer(buffer.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to get buffer, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        win32::com_ptr<IMF2DBuffer> buffer_2d;
        LONG source_pitch = m_frame_pitch;
        BYTE* src_data = nullptr;
        bool using_2d_buffer = false;
        
        if (SUCCEEDED(buffer->QueryInterface(IID_PPV_ARGS(buffer_2d.put())))) {
            hr = buffer_2d->Lock2D(&src_data, &source_pitch);
            if (SUCCEEDED(hr)) {
                using_2d_buffer = true;
            }
        }
        
        if (!using_2d_buffer) {
            DWORD max_length = 0, current_length = 0;
            hr = buffer->Lock(&src_data, &max_length, &current_length);
            if (FAILED(hr)) {
                return false;
            }
        }
        
        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = m_device_context->Map(m_texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) {
            if (using_2d_buffer) {
                buffer_2d->Unlock2D();
            } else {
                buffer->Unlock();
            }
            Logger::error("[core] [VideoDecoder] Failed to map texture, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
        const uint8_t* src = src_data;
        const size_t copy_size = m_frame_pitch;
        
        GUID format = GUID_NULL;
        bool force_opaque_alpha = false;
        if (m_media_type && SUCCEEDED(m_media_type->GetGUID(MF_MT_SUBTYPE, &format))) {
            if (format == MFVideoFormat_RGB32) {
                force_opaque_alpha = true;
            }
        }
        
        static bool first_copy = true;
        if (first_copy) {
            Logger::info("[core] [VideoDecoder] Texture update: source_pitch={}, mapped.RowPitch={}, copy_size={}, height={}, force_alpha={}",
                        source_pitch, mapped.RowPitch, copy_size, m_target_size.y, force_opaque_alpha);
            bool all_zero = true;
            for (size_t i = 0; i < std::min<size_t>(16, copy_size); ++i) {
                if (src[i] != 0) {
                    all_zero = false;
                    break;
                }
            }
            Logger::info("[core] [VideoDecoder] First 16 bytes all zero: {}", all_zero);
            first_copy = false;
        }
        
        if (!force_opaque_alpha) {
            for (uint32_t y = 0; y < m_target_size.y; ++y) {
                memcpy(dst, src, copy_size);
                dst += mapped.RowPitch;
                src += source_pitch;
            }
        } else {
            for (uint32_t y = 0; y < m_target_size.y; ++y) {
                memcpy(dst, src, copy_size);
                for (uint32_t x = 0; x < m_target_size.x; ++x) {
                    dst[x * 4 + 3] = 0xFF;
                }
                dst += mapped.RowPitch;
                src += source_pitch;
            }
        }
        
        m_device_context->Unmap(m_texture.get(), 0);
        
        if (using_2d_buffer) {
            buffer_2d->Unlock2D();
        } else {
            buffer->Unlock();
        }
        
        return true;
    }
    
}
