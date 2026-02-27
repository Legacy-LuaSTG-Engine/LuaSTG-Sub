#include "d3d11/VideoDecoder.hpp"
#include "d3d11/VideoDecoderConfig.hpp"
#include "core/Configuration.hpp"
#include "core/FileSystem.hpp"
#include "core/Logger.hpp"
#include "utf8.hpp"
#include <wil/resource.h>
#include <mferror.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <algorithm>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace {
    using std::string_view_literals::operator ""sv;
    using Config = core::VideoDecoderConfig;
    
    class MFInitializer {
    public:
        static MFInitializer& getInstance() {
            static MFInitializer instance;
            return instance;
        }
        
        bool isInitialized() const { return m_initialized; }
        
    private:
        MFInitializer() {
            m_initialized = win32::check_hresult_as_boolean(
                MFStartup(MF_VERSION, MFSTARTUP_FULL),
                "MFStartup"sv
            );
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
        return open(path, VideoOpenOptions{});
    }

    bool VideoDecoder::open(StringView path, VideoOpenOptions const& options) {
        if (!m_initialized) {
            Logger::error("[core] [VideoDecoder] Not initialized");
            return false;
        }
        
        close();
        
        if (!loadVideoFile(path)) return false;
        if (!configureHardwareAcceleration()) return false;
        if (!selectVideoStream(options.video_stream_index)) return false;
        if (!negotiateOutputFormat(options)) return false;
        if (!extractVideoProperties()) return false;
        if (!createResources()) return false;
        if (!loadFirstFrame()) return false;

        m_last_open_path.assign(path.data(), path.size());
        m_last_open_options = options;

        setLooping(options.looping);
        if (options.loop_duration > 0.0) {
            setLoopRange(options.loop_end, options.loop_duration);
        }

        Logger::info("[core] [VideoDecoder] Opened video: {}x{}, duration: {:.2f}s", 
                    m_target_size.x, m_target_size.y, m_duration);

        return true;
    }

    void VideoDecoder::getVideoStreams(void (*callback)(VideoStreamInfo const&, void*), void* userdata) const {
        if (!m_source_reader || !callback) return;
        
        wil::unique_prop_variant var;
        double duration_sec = 0.0;
        if (SUCCEEDED(m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))) {
            duration_sec = var.hVal.QuadPart / 10000000.0;
        }
        
        for (DWORD si = 0; si < Config::kMaxStreams; ++si) {
            win32::com_ptr<IMFMediaType> mt;
            if (FAILED(m_source_reader->GetNativeMediaType(si, 0, mt.put()))) continue;
            
            GUID major = GUID_NULL;
            if (FAILED(mt->GetGUID(MF_MT_MAJOR_TYPE, &major)) || major != MFMediaType_Video) continue;
            
            VideoStreamInfo info{};
            info.index = si;
            info.duration_seconds = duration_sec;
            
            UINT32 w = 0, h = 0;
            if (SUCCEEDED(MFGetAttributeSize(mt.get(), MF_MT_FRAME_SIZE, &w, &h))) {
                info.width = w;
                info.height = h;
            }
            
            UINT32 num = 0, den = 0;
            if (SUCCEEDED(MFGetAttributeRatio(mt.get(), MF_MT_FRAME_RATE, &num, &den)) && den > 0) {
                info.fps = static_cast<double>(num) / static_cast<double>(den);
            }
            
            callback(info, userdata);
        }
    }

    void VideoDecoder::getAudioStreams(void (*callback)(AudioStreamInfo const&, void*), void* userdata) const {
        if (!m_source_reader || !callback) return;
        
        wil::unique_prop_variant var;
        double duration_sec = 0.0;
        if (SUCCEEDED(m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))) {
            duration_sec = var.hVal.QuadPart / 10000000.0;
        }
        
        for (DWORD si = 0; si < Config::kMaxStreams; ++si) {
            win32::com_ptr<IMFMediaType> mt;
            if (FAILED(m_source_reader->GetNativeMediaType(si, 0, mt.put()))) continue;
            
            GUID major = GUID_NULL;
            if (FAILED(mt->GetGUID(MF_MT_MAJOR_TYPE, &major)) || major != MFMediaType_Audio) continue;
            
            AudioStreamInfo info{};
            info.index = si;
            info.duration_seconds = duration_sec;
            info.channels = (UINT32)MFGetAttributeUINT32(mt.get(), MF_MT_AUDIO_NUM_CHANNELS, 0);
            info.sample_rate = MFGetAttributeUINT32(mt.get(), MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
            
            callback(info, userdata);
        }
    }

    bool VideoDecoder::reopen(VideoOpenOptions const& options) {
        if (m_last_open_path.empty()) {
            Logger::error("[core] [VideoDecoder] reopen: no previous path (open was never successful)");
            return false;
        }
        return open(StringView(m_last_open_path), options);
    }
    
    void VideoDecoder::close() {
        m_source_reader.reset();
        m_byte_stream.reset();
        m_media_type.reset();
        m_texture.reset();
        m_shader_resource_view.reset();
        m_device_context.reset();
        m_video_processor.reset();
        m_video_processor_enum.reset();
        m_video_context.reset();
        m_video_device.reset();
        
        m_output_format = OutputFormat::ARGB32;
        m_video_size = Vector2U{};
        m_target_size = Vector2U{};
        m_duration = 0.0;
        m_current_time = 0.0;
        m_last_requested_time = -1.0;
        m_frame_interval = 1.0 / Config::kDefaultFrameRateNum;
        m_frame_rate_num = Config::kDefaultFrameRateNum;
        m_frame_rate_den = Config::kDefaultFrameRateDen;
        m_frame_pitch = 0;
        m_video_stream_index = 0;
        m_loop_state = LoopState{};
        m_first_texture_update_logged = false;
    }

    void VideoDecoder::setLoopRange(double end_sec, double duration_sec) {
        if (duration_sec > 0.0 && end_sec >= duration_sec) {
            m_loop_state.start = end_sec - duration_sec;
            double cap = m_duration > 0.0 ? m_duration : end_sec;
            m_loop_state.end = (std::min)(end_sec, cap);
            m_loop_state.has_range = (m_loop_state.end > m_loop_state.start);
            if (!m_loop_state.has_range) { 
                m_loop_state.start = 0.0; 
                m_loop_state.end = m_duration; 
            }
        } else {
            m_loop_state.has_range = false;
            m_loop_state.start = 0.0;
            m_loop_state.end = m_duration;
        }
    }

    void VideoDecoder::getLoopRange(double* end_sec, double* duration_sec) const noexcept {
        if (end_sec) *end_sec = m_loop_state.has_range ? m_loop_state.end : m_duration;
        if (duration_sec) *duration_sec = m_loop_state.has_range ? (m_loop_state.end - m_loop_state.start) : m_duration;
    }
    
    bool VideoDecoder::seek(double time_in_seconds) {
        if (!hasVideo()) {
            return false;
        }
        
        time_in_seconds = std::clamp(time_in_seconds, 0.0, m_duration);
        
        wil::unique_prop_variant var;
        var.vt = VT_I8;
        var.hVal.QuadPart = static_cast<LONGLONG>(time_in_seconds * 10000000.0);
        
        HRESULT const hr = m_source_reader->SetCurrentPosition(GUID_NULL, var);
        
        if (!win32::check_hresult_as_boolean(hr, "IMFSourceReader::SetCurrentPosition"sv)) {
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
        
        auto const loop_begin = m_loop_state.has_range ? m_loop_state.start : 0.0;
        auto const loop_end = m_loop_state.has_range ? m_loop_state.end : m_duration;
        auto const loop_len = loop_end - loop_begin;

        if (time_in_seconds >= m_duration) {
            if (m_looping) {
                if (loop_end > m_duration && time_in_seconds < loop_end) {
                    if (m_current_time < m_duration - Config::kEndEpsilon) {
                        double const last_frame_time = std::max(0.0, m_duration - Config::kEndEpsilon);
                        readFrameAtTime(last_frame_time);
                        m_current_time = m_duration;
                    }
                    return true;
                }
                if (loop_len > 0.0)
                    time_in_seconds = loop_begin + std::fmod(time_in_seconds - loop_begin, loop_len);
                else
                    time_in_seconds = std::fmod(time_in_seconds, m_duration);
            } else {
                if (m_current_time >= m_duration - Config::kEndEpsilon) {
                    return true;
                }
                double const last_frame_time = std::max(0.0, m_duration - Config::kEndEpsilon);
                bool const ok = readFrameAtTime(last_frame_time);
                m_current_time = m_duration;
                return ok;
            }
        } else if (time_in_seconds >= loop_end && m_looping && loop_len > 0.0) {
            time_in_seconds = loop_begin + std::fmod(time_in_seconds - loop_begin, loop_len);
        } else if (time_in_seconds < loop_begin) {
            if (m_looping && loop_len > 0.0) {
                double t = time_in_seconds - loop_begin;
                t = loop_begin + std::fmod(t, loop_len);
                if (t < loop_begin) t += loop_len;
                time_in_seconds = t;
            } else {
                time_in_seconds = loop_begin;
            }
        } else if (time_in_seconds < 0.0) {
            if (m_looping) {
                if (loop_len > 0.0) {
                    double t = std::fmod(time_in_seconds - loop_begin, loop_len);
                    if (t < 0.0) t += loop_len;
                    time_in_seconds = loop_begin + t;
                } else {
                    double t = std::fmod(time_in_seconds, m_duration);
                    if (t < 0.0) t += m_duration;
                    time_in_seconds = t;
                }
            } else {
                time_in_seconds = 0.0;
            }
        }
        
        double const frame_tolerance = m_frame_interval * 0.5;
        double const seek_threshold = (std::max)(0.1, 16.0 * m_frame_interval);
        int const max_catch_up_frames = (std::min)(16, (std::max)(1, static_cast<int>(0.2 / m_frame_interval)));

        bool const is_backward = (m_last_requested_time >= 0.0) && (time_in_seconds + Config::kBackwardTolerance < m_last_requested_time);
        bool const large_jump = (m_current_time + seek_threshold < time_in_seconds);

        m_last_requested_time = time_in_seconds;

        if (is_backward || large_jump) {
            return readFrameAtTime(time_in_seconds);
        }

        if (m_current_time + frame_tolerance >= time_in_seconds) {
            return true;
        }

        for (int i = 0; i < max_catch_up_frames; ++i) {
            if (!readNextFrame()) {
                return false;
            }
            if (m_current_time + Config::kTimeEpsilon >= time_in_seconds) {
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
            (DWORD)m_video_stream_index,
            0,
            nullptr,
            &stream_flags,
            &timestamp,
            sample.put()
        );

        if (!win32::check_hresult_as_boolean(hr, "IMFSourceReader::ReadSample"sv)) {
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

        for (int i = 0; i < Config::kMaxDecodeFramesAfterSeek; ++i) {
            if (!readNextFrame()) {
                return false;
            }
            if (m_current_time + Config::kTimeEpsilon >= time_in_seconds) {
                return true;
            }
        }

        return false;
    }
    
    void VideoDecoder::onGraphicsDeviceCreate() {
        if (hasVideo()) {
            createTexture();
            if (m_output_format == OutputFormat::NV12) {
                createVideoProcessor();
            }
        }
    }
    
    void VideoDecoder::onGraphicsDeviceDestroy() {
        close();
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
        if (m_output_format == OutputFormat::NV12) {
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
        } else {
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }
        desc.MiscFlags = 0;
        
        HRESULT hr = d3d_device->CreateTexture2D(&desc, nullptr, m_texture.put());
        if (!win32::check_hresult_as_boolean(hr, "ID3D11Device::CreateTexture2D"sv)) {
            return false;
        }
        
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        srv_desc.Texture2D.MostDetailedMip = 0;
        
        hr = d3d_device->CreateShaderResourceView(m_texture.get(), &srv_desc, m_shader_resource_view.put());
        if (!win32::check_hresult_as_boolean(hr, "ID3D11Device::CreateShaderResourceView"sv)) {
            m_texture.reset();
            return false;
        }
        
        d3d_device->GetImmediateContext(m_device_context.put());
        
        win32::com_ptr<ID3D11Multithread> multithread;
        if (SUCCEEDED(m_device_context->QueryInterface(IID_PPV_ARGS(multithread.put())))) {
            multithread->SetMultithreadProtected(TRUE);
        }
        
        return true;
    }
    
    bool VideoDecoder::createVideoProcessor() {
        if (!m_device || m_target_size.x == 0 || m_target_size.y == 0 || !m_texture) {
            return false;
        }
        
        auto d3d_device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!d3d_device) {
            return false;
        }
        
        HRESULT hr = d3d_device->QueryInterface(IID_PPV_ARGS(m_video_device.put()));
        if (!win32::check_hresult_as_boolean(hr, "ID3D11Device::QueryInterface(ID3D11VideoDevice)"sv)) {
            return false;
        }
        
        hr = m_device_context->QueryInterface(IID_PPV_ARGS(m_video_context.put()));
        if (!win32::check_hresult_as_boolean(hr, "ID3D11DeviceContext::QueryInterface(ID3D11VideoContext)"sv)) {
            return false;
        }
        
        D3D11_VIDEO_PROCESSOR_CONTENT_DESC content_desc = {};
        content_desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
        UINT input_fps_num = m_frame_rate_num;
        UINT input_fps_den = m_frame_rate_den;
        if (input_fps_num == 0 || input_fps_den == 0) {
            input_fps_num = 30;
            input_fps_den = 1;
        }
        content_desc.InputFrameRate.Numerator = input_fps_num;
        content_desc.InputFrameRate.Denominator = input_fps_den;
        content_desc.InputWidth = m_video_size.x;
        content_desc.InputHeight = m_video_size.y;
        content_desc.OutputFrameRate.Numerator = input_fps_num;
        content_desc.OutputFrameRate.Denominator = input_fps_den;
        content_desc.OutputWidth = m_target_size.x;
        content_desc.OutputHeight = m_target_size.y;
        content_desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;
        
        hr = m_video_device->CreateVideoProcessorEnumerator(&content_desc, m_video_processor_enum.put());
        if (!win32::check_hresult_as_boolean(hr, "ID3D11VideoDevice::CreateVideoProcessorEnumerator"sv)) {
            return false;
        }
        
        hr = m_video_device->CreateVideoProcessor(m_video_processor_enum.get(), 0, m_video_processor.put());
        if (!win32::check_hresult_as_boolean(hr, "ID3D11VideoDevice::CreateVideoProcessor"sv)) {
            return false;
        }
        
        Logger::info("[core] [VideoDecoder] Video Processor created for NV12->BGRA conversion");
        return true;
    }
    
    bool VideoDecoder::updateTextureFromNV12Sample(IMFSample* sample) {
        if (!m_texture || !sample) return false;
        return updateTextureFromNV12SampleTo(sample, m_texture.get());
    }
    
    bool VideoDecoder::updateTextureFromNV12SampleTo(IMFSample* sample, ID3D11Texture2D* output_texture) {
        if (!output_texture || !sample || !m_device_context || !m_video_device || !m_video_context
            || !m_video_processor || !m_video_processor_enum) {
            return false;
        }
        
        DWORD buffer_count = 0;
        HRESULT hr = sample->GetBufferCount(&buffer_count);
        if (!win32::check_hresult_as_boolean(hr, "IMFSample::GetBufferCount"sv) || buffer_count == 0) {
            return false;
        }
        
        win32::com_ptr<IMFMediaBuffer> buffer;
        if (!win32::check_hresult_as_boolean(
            sample->GetBufferByIndex(0, buffer.put()),
            "IMFSample::GetBufferByIndex"sv
        )) {
            return false;
        }
        
        win32::com_ptr<IMFDXGIBuffer> dxgi_buffer;
        if (!win32::check_hresult_as_boolean(
            buffer->QueryInterface(IID_PPV_ARGS(dxgi_buffer.put())),
            "IMFMediaBuffer::QueryInterface(IMFDXGIBuffer)"sv
        )) {
            Logger::error("[core] [VideoDecoder] NV12 sample does not have DXGI buffer");
            return false;
        }
        
        win32::com_ptr<ID3D11Texture2D> nv12_texture;
        hr = dxgi_buffer->GetResource(IID_PPV_ARGS(nv12_texture.put()));
        if (!win32::check_hresult_as_boolean(hr, "IMFDXGIBuffer::GetResource"sv) || !nv12_texture) {
            return false;
        }
        
        D3D11_TEXTURE2D_DESC nv12_desc{};
        nv12_texture->GetDesc(&nv12_desc);
        if (nv12_desc.Format != DXGI_FORMAT_NV12) {
            Logger::error("[core] [VideoDecoder] Expected NV12 format, got {:x}", (uint32_t)nv12_desc.Format);
            return false;
        }
        
        UINT subresource_index = 0;
        dxgi_buffer->GetSubresourceIndex(&subresource_index);
        
        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC input_view_desc = {};
        input_view_desc.FourCC = 0;
        input_view_desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
        input_view_desc.Texture2D.ArraySlice = subresource_index / std::max(1u, nv12_desc.MipLevels);
        input_view_desc.Texture2D.MipSlice = subresource_index % std::max(1u, nv12_desc.MipLevels);
        
        win32::com_ptr<ID3D11VideoProcessorInputView> input_view;
        hr = m_video_device->CreateVideoProcessorInputView(
            nv12_texture.get(), m_video_processor_enum.get(), &input_view_desc, input_view.put());
        if (!win32::check_hresult_as_boolean(hr, "ID3D11VideoDevice::CreateVideoProcessorInputView"sv)) {
            input_view_desc.Texture2D.ArraySlice = 0;
            input_view_desc.Texture2D.MipSlice = 0;
            hr = m_video_device->CreateVideoProcessorInputView(
                nv12_texture.get(), m_video_processor_enum.get(), &input_view_desc, input_view.put());
        }
        if (!win32::check_hresult_as_boolean(hr, "ID3D11VideoDevice::CreateVideoProcessorInputView (retry)"sv)) {
            return false;
        }
        
        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc = {};
        output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
        output_view_desc.Texture2D.MipSlice = 0;
        
        win32::com_ptr<ID3D11VideoProcessorOutputView> output_view;
        if (!win32::check_hresult_as_boolean(
            m_video_device->CreateVideoProcessorOutputView(
                output_texture, m_video_processor_enum.get(), &output_view_desc, output_view.put()),
            "ID3D11VideoDevice::CreateVideoProcessorOutputView"sv
        )) {
            return false;
        }
        
        D3D11_VIDEO_PROCESSOR_STREAM stream = {};
        stream.Enable = TRUE;
        stream.OutputIndex = 0;
        stream.InputFrameOrField = 0;
        stream.PastFrames = 0;
        stream.FutureFrames = 0;
        stream.pInputSurface = input_view.get();
        
        if (!win32::check_hresult_as_boolean(
            m_video_context->VideoProcessorBlt(m_video_processor.get(), output_view.get(), 0, 1, &stream),
            "ID3D11VideoContext::VideoProcessorBlt"sv
        )) {
            return false;
        }
        
        return true;
    }
    
    bool VideoDecoder::updateTextureFromSample(IMFSample* sample) {
        if (!m_texture || !sample || !m_device_context) {
            return false;
        }
        std::lock_guard<std::mutex> lock(m_device_context_mutex);
        if (m_output_format == OutputFormat::NV12 && m_video_processor) {
            return updateTextureFromNV12SampleTo(sample, m_texture.get());
        }
        
        DWORD buffer_count = 0;
        HRESULT hr = sample->GetBufferCount(&buffer_count);
        if (SUCCEEDED(hr) && buffer_count > 0) {
            win32::com_ptr<IMFMediaBuffer> buffer;
            hr = sample->GetBufferByIndex(0, buffer.put());
            if (SUCCEEDED(hr)) {
                win32::com_ptr<IMFDXGIBuffer> dxgi_buffer;
                if (SUCCEEDED(buffer->QueryInterface(IID_PPV_ARGS(dxgi_buffer.put())))) {
                    win32::com_ptr<ID3D11Texture2D> source_texture;
                    hr = dxgi_buffer->GetResource(IID_PPV_ARGS(source_texture.put()));
                    if (SUCCEEDED(hr) && source_texture) {
                        D3D11_TEXTURE2D_DESC src_desc{};
                        source_texture->GetDesc(&src_desc);
                        D3D11_TEXTURE2D_DESC dst_desc{};
                        m_texture->GetDesc(&dst_desc);
                        if (src_desc.Width == dst_desc.Width && src_desc.Height == dst_desc.Height
                            && src_desc.Format == dst_desc.Format) {
                            m_device_context->CopyResource(m_texture.get(), source_texture.get());
                            return true;
                        }
                    }
                }
            }
        }
        
        win32::com_ptr<IMFMediaBuffer> buffer;
        if (!win32::check_hresult_as_boolean(
            sample->ConvertToContiguousBuffer(buffer.put()),
            "IMFSample::ConvertToContiguousBuffer"sv
        )) {
            return false;
        }
        
        win32::com_ptr<IMF2DBuffer> buffer_2d;
        LONG source_pitch = m_frame_pitch;
        BYTE* src_data = nullptr;
        bool using_2d_buffer = false;
        
        if (SUCCEEDED(buffer->QueryInterface(IID_PPV_ARGS(buffer_2d.put())))) {
            if (SUCCEEDED(buffer_2d->Lock2D(&src_data, &source_pitch))) {
                using_2d_buffer = true;
            }
        }
        
        if (!using_2d_buffer) {
            DWORD max_length = 0, current_length = 0;
            if (!win32::check_hresult_as_boolean(
                buffer->Lock(&src_data, &max_length, &current_length),
                "IMFMediaBuffer::Lock"sv
            )) {
                return false;
            }
        }
        
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (!win32::check_hresult_as_boolean(
            m_device_context->Map(m_texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
            "ID3D11DeviceContext::Map"sv
        )) {
            if (using_2d_buffer) {
                buffer_2d->Unlock2D();
            } else {
                buffer->Unlock();
            }
            return false;
        }
        
        uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
        const uint8_t* src = src_data;
        const uint32_t src_w = m_video_size.x;
        const uint32_t src_h = m_video_size.y;
        const uint32_t dst_w = m_target_size.x;
        const uint32_t dst_h = m_target_size.y;
        const bool need_scale = (src_w != dst_w || src_h != dst_h);
        
        GUID format = GUID_NULL;
        bool force_opaque_alpha = false;
        if (m_media_type && SUCCEEDED(m_media_type->GetGUID(MF_MT_SUBTYPE, &format))) {
            if (format == MFVideoFormat_RGB32) {
                force_opaque_alpha = true;
            }
        }
        
        if (!m_first_texture_update_logged) [[unlikely]] {
            Logger::info("[core] [VideoDecoder] Texture update: source={}x{} pitch={}, dest={}x{} pitch={}, force_alpha={}, scale={}",
                        src_w, src_h, source_pitch, dst_w, dst_h, mapped.RowPitch, force_opaque_alpha, need_scale);
            m_first_texture_update_logged = true;
        }
        
        if (!need_scale) {
            const size_t copy_size = m_frame_pitch;
            if (!force_opaque_alpha) {
                for (uint32_t y = 0; y < dst_h; ++y) {
                    memcpy(dst, src, copy_size);
                    dst += mapped.RowPitch;
                    src += source_pitch;
                }
            } else {
                for (uint32_t y = 0; y < dst_h; ++y) {
                    memcpy(dst, src, copy_size);
                    for (uint32_t x = 0; x < dst_w; ++x) {
                        dst[x * 4 + 3] = 0xFF;
                    }
                    dst += mapped.RowPitch;
                    src += source_pitch;
                }
            }
        } else {
            for (uint32_t y_dst = 0; y_dst < dst_h; ++y_dst) {
                uint32_t y_src = (src_h > 1) ? (y_dst * src_h / dst_h) : 0;
                if (y_src >= src_h) y_src = src_h - 1;
                const uint8_t* row_src = src + static_cast<size_t>(y_src) * source_pitch;
                for (uint32_t x_dst = 0; x_dst < dst_w; ++x_dst) {
                    uint32_t x_src = (src_w > 1) ? (x_dst * src_w / dst_w) : 0;
                    if (x_src >= src_w) x_src = src_w - 1;
                    const uint8_t* px = row_src + static_cast<size_t>(x_src) * 4;
                    dst[0] = px[0];
                    dst[1] = px[1];
                    dst[2] = px[2];
                    dst[3] = force_opaque_alpha ? 0xFF : px[3];
                    dst += 4;
                }
                dst += mapped.RowPitch - static_cast<size_t>(dst_w) * 4;
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
