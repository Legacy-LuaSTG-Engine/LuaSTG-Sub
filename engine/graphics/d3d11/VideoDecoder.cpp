#include "d3d11/VideoDecoder.hpp"
#include "core/Configuration.hpp"
#include "core/FileSystem.hpp"
#include "core/Logger.hpp"
#include "utf8.hpp"
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
#pragma comment(lib, "shlwapi.lib")

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
    
    // Maximum number of streams to enumerate (video/audio)
    constexpr DWORD kMaxStreams = 16;
    
    // Maximum number of frames to decode after seeking to reach the requested timestamp
    constexpr int kMaxDecodeFramesAfterSeek = 360;
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
        
        SmartReference<IData> file_data;
        if (!FileSystemManager::readFile(path, file_data.put())) {
            Logger::error("[core] [VideoDecoder] Failed to read video file: {}", path);
            return false;
        }
        
        win32::com_ptr<IStream> mem_stream;
        mem_stream.attach(static_cast<IStream*>(SHCreateMemStream(
            static_cast<BYTE const*>(file_data->data()),
            static_cast<UINT>(file_data->size()))));
        if (!mem_stream) {
            Logger::error("[core] [VideoDecoder] Failed to create memory stream");
            return false;
        }
        
        win32::com_ptr<IMFByteStream> byte_stream;
        HRESULT hr = MFCreateMFByteStreamOnStream(mem_stream.get(), byte_stream.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create byte stream, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        m_byte_stream = std::move(byte_stream);
        
        auto d3d_device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!d3d_device) {
            Logger::error("[core] [VideoDecoder] Failed to get D3D11 device");
            return false;
        }
        
        win32::com_ptr<ID3D11DeviceContext> temp_context;
        d3d_device->GetImmediateContext(temp_context.put());
        if (temp_context) {
            win32::com_ptr<ID3D11Multithread> multithread;
            if (SUCCEEDED(temp_context->QueryInterface(IID_PPV_ARGS(multithread.put())))) {
                multithread->SetMultithreadProtected(TRUE);
                Logger::info("[core] [VideoDecoder] Enabled D3D11 multithread protection");
            } else {
                Logger::warn("[core] [VideoDecoder] Failed to enable D3D11 multithread protection");
            }
        }
        
        win32::com_ptr<ID3D11VideoDevice> video_device;
        hr = d3d_device->QueryInterface(IID_PPV_ARGS(video_device.put()));
        if (SUCCEEDED(hr)) {
            Logger::info("[core] [VideoDecoder] D3D11 device supports video decoding");
        } else {
            Logger::warn("[core] [VideoDecoder] D3D11 device does not support video decoding (hr = {:#x})", (uint32_t)hr);
        }
        
        win32::com_ptr<IMFAttributes> attributes;
        hr = MFCreateAttributes(attributes.put(), 3);
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create attributes, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        win32::com_ptr<IMFDXGIDeviceManager> dxgi_device_manager;
        UINT dxgi_reset_token = 0;
        bool use_hardware_accel = false;
        
        bool const allow_hw_decode = core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowHardwareVideoDecode();
        hr = MFCreateDXGIDeviceManager(&dxgi_reset_token, dxgi_device_manager.put());
        if (SUCCEEDED(hr)) {
            hr = dxgi_device_manager->ResetDevice(d3d_device, dxgi_reset_token);
            if (SUCCEEDED(hr)) {
                if (video_device && allow_hw_decode) {
                    hr = attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxgi_device_manager.get());
                    if (SUCCEEDED(hr)) {
                        use_hardware_accel = true;
                        Logger::info("[core] [VideoDecoder] Attempting D3D11 hardware acceleration (IMFDXGIDeviceManager)");
                    } else {
                        Logger::warn("[core] [VideoDecoder] Failed to set D3D manager attribute, hr = {:#x}", (uint32_t)hr);
                    }
                } else if (!allow_hw_decode) {
                    Logger::info("[core] [VideoDecoder] Hardware video decode disabled by config");
                } else {
                    Logger::warn("[core] [VideoDecoder] Skipping hardware acceleration - device does not support video");
                }
            } else {
                Logger::warn("[core] [VideoDecoder] Failed to reset DXGI device, hr = {:#x}", (uint32_t)hr);
            }
        } else {
            Logger::warn("[core] [VideoDecoder] Failed to create DXGI device manager, hr = {:#x}", (uint32_t)hr);
        }
        
        if (!use_hardware_accel) {
            hr = attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
            if (FAILED(hr)) {
                Logger::warn("[core] [VideoDecoder] Failed to set hardware transforms attribute, hr = {:#x}", (uint32_t)hr);
            }
            
            hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
            if (FAILED(hr)) {
                Logger::warn("[core] [VideoDecoder] Failed to set video processing attribute, hr = {:#x}", (uint32_t)hr);
            }
        }
        
        hr = MFCreateSourceReaderFromByteStream(m_byte_stream.get(), attributes.get(), m_source_reader.put());
        if (FAILED(hr)) {
            if (use_hardware_accel && hr == E_INVALIDARG) {
                Logger::warn("[core] [VideoDecoder] Hardware acceleration failed (hr = {:#x}), retrying without D3D manager", (uint32_t)hr);
                attributes->DeleteItem(MF_SOURCE_READER_D3D_MANAGER);
                dxgi_device_manager.reset();
                use_hardware_accel = false;
                
                hr = MFCreateSourceReaderFromByteStream(m_byte_stream.get(), attributes.get(), m_source_reader.put());
            }
            
            if (FAILED(hr)) {
                Logger::error("[core] [VideoDecoder] Failed to create source reader, hr = {:#x}", (uint32_t)hr);
                return false;
            } else {
                Logger::info("[core] [VideoDecoder] Falling back to software decoding");
            }
        } else if (use_hardware_accel) {
            Logger::info("[core] [VideoDecoder] Using D3D11 hardware acceleration (IMFDXGIDeviceManager)");
        }
        
        m_video_stream_index = options.video_stream_index;
        if (m_video_stream_index == static_cast<uint32_t>(-1)) {
            for (DWORD i = 0; i < kMaxStreams; ++i) {
                win32::com_ptr<IMFMediaType> mt;
                if (FAILED(m_source_reader->GetNativeMediaType(i, 0, mt.put()))) continue;
                GUID major = GUID_NULL;
                if (FAILED(mt->GetGUID(MF_MT_MAJOR_TYPE, &major)) || major != MFMediaType_Video) continue;
                m_video_stream_index = i;
                Logger::info("[core] [VideoDecoder] Auto-selected video stream index {}", (unsigned)i);
                break;
            }
            if (m_video_stream_index == static_cast<uint32_t>(-1)) {
                Logger::error("[core] [VideoDecoder] No video stream found in file");
                return false;
            }
        }
        
        // 流选择：仅启用指定的视频流
        for (DWORD i = 0; i < kMaxStreams; ++i) {
            BOOL const select = (i == m_video_stream_index);
            m_source_reader->SetStreamSelection(i, select);
        }
        
        win32::com_ptr<IMFMediaType> partial_media_type;
        hr = m_source_reader->GetNativeMediaType((DWORD)m_video_stream_index, 0, partial_media_type.put());
        if (FAILED(hr)) {
            Logger::warn("[core] [VideoDecoder] Failed to get native media type, trying current type, hr = {:#x}", (uint32_t)hr);
            hr = m_source_reader->GetCurrentMediaType((DWORD)m_video_stream_index, partial_media_type.put());
            if (FAILED(hr)) {
                Logger::warn("[core] [VideoDecoder] Failed to get current media type, hr = {:#x}", (uint32_t)hr);
            }
        }
        
        if (partial_media_type && use_hardware_accel) {
            GUID native_subtype = GUID_NULL;
            if (SUCCEEDED(partial_media_type->GetGUID(MF_MT_SUBTYPE, &native_subtype))) {
                const char* format_name = "Unknown";
                if (native_subtype == MFVideoFormat_NV12) {
                    format_name = "NV12";
                } else if (native_subtype == MFVideoFormat_ARGB32) {
                    format_name = "ARGB32";
                } else if (native_subtype == MFVideoFormat_RGB32) {
                    format_name = "RGB32";
                }
                Logger::info("[core] [VideoDecoder] Native hardware decoder format: {} ({:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x})",
                    format_name,
                    native_subtype.Data1, native_subtype.Data2, native_subtype.Data3,
                    native_subtype.Data4[0], native_subtype.Data4[1],
                    native_subtype.Data4[2], native_subtype.Data4[3],
                    native_subtype.Data4[4], native_subtype.Data4[5],
                    native_subtype.Data4[6], native_subtype.Data4[7]);
            }
        }
        
        win32::com_ptr<IMFMediaType> media_type;
        hr = MFCreateMediaType(media_type.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create media type, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to set major type, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        if (use_hardware_accel && partial_media_type) {
            GUID native_subtype = GUID_NULL;
            if (SUCCEEDED(partial_media_type->GetGUID(MF_MT_SUBTYPE, &native_subtype))) {
                if (native_subtype == MFVideoFormat_H264) {
                    Logger::info("[core] [VideoDecoder] Hardware decoder outputs compressed format, enumerating supported output formats");
                    for (DWORD i = 0; ; ++i) {
                        win32::com_ptr<IMFMediaType> output_type;
                        hr = m_source_reader->GetNativeMediaType((DWORD)m_video_stream_index, i, output_type.put());
                        if (FAILED(hr)) {
                            break;
                        }
                        GUID output_subtype = GUID_NULL;
                        if (SUCCEEDED(output_type->GetGUID(MF_MT_SUBTYPE, &output_subtype))) {
                            const char* format_name = "Unknown";
                            if (output_subtype == MFVideoFormat_NV12) format_name = "NV12";
                            else if (output_subtype == MFVideoFormat_ARGB32) format_name = "ARGB32";
                            else if (output_subtype == MFVideoFormat_RGB32) format_name = "RGB32";
                            Logger::info("[core] [VideoDecoder]   Output format {}: {}", i, format_name);
                        }
                    }
                }
            }
        }
        
        if (partial_media_type) {
            UINT32 width = 0, height = 0;
            if (SUCCEEDED(MFGetAttributeSize(partial_media_type.get(), MF_MT_FRAME_SIZE, &width, &height))) {
                UINT32 out_w = options.output_width > 0 ? options.output_width : width;
                UINT32 out_h = options.output_height > 0 ? options.output_height : height;
                if (out_w != width || out_h != height) {
                    Logger::info("[core] [VideoDecoder] Requesting output resolution: {}x{} (original: {}x{})", out_w, out_h, width, height);
                }
                hr = MFSetAttributeSize(media_type.get(), MF_MT_FRAME_SIZE, out_w, out_h);
                if (FAILED(hr)) {
                    Logger::warn("[core] [VideoDecoder] Failed to set output resolution {}x{}, using original {}x{}, hr = {:#x}", 
                        out_w, out_h, width, height, (uint32_t)hr);
                    hr = MFSetAttributeSize(media_type.get(), MF_MT_FRAME_SIZE, width, height);
                    if (FAILED(hr)) {
                        Logger::error("[core] [VideoDecoder] Failed to set original frame size, hr = {:#x}", (uint32_t)hr);
                    }
                }
            }
            
            MFVideoArea area = {};
            if (SUCCEEDED(partial_media_type->GetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(area), nullptr))) {
                media_type->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(area));
            }
        }
        
        hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to set ARGB32 subtype, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type.get());
        
        if (FAILED(hr)) {
            Logger::info("[core] [VideoDecoder] ARGB32 not supported (hr = {:#x}), trying RGB32", (uint32_t)hr);
            hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
            if (FAILED(hr)) {
                Logger::error("[core] [VideoDecoder] Failed to set RGB32 subtype, hr = {:#x}", (uint32_t)hr);
                return false;
            }
            hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type.get());
            
            if (FAILED(hr)) {
                Logger::info("[core] [VideoDecoder] RGB32 not supported (hr = {:#x}), trying NV12 (hardware native)", (uint32_t)hr);
                hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
                if (FAILED(hr)) {
                    Logger::error("[core] [VideoDecoder] Failed to set NV12 subtype, hr = {:#x}", (uint32_t)hr);
                    return false;
                }
                hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type.get());
                bool succeeded_via_retry = false;
                if (FAILED(hr)) {
                    UINT32 req_w = 0, req_h = 0;
                    MFGetAttributeSize(media_type.get(), MF_MT_FRAME_SIZE, &req_w, &req_h);
                    UINT32 orig_w = 0, orig_h = 0;
                    bool can_retry_original = partial_media_type
                        && SUCCEEDED(MFGetAttributeSize(partial_media_type.get(), MF_MT_FRAME_SIZE, &orig_w, &orig_h))
                        && (req_w != orig_w || req_h != orig_h);
                    if (can_retry_original) {
                        Logger::info("[core] [VideoDecoder] Scaled resolution {}x{} not supported by decoder, retrying with original {}x{}",
                            req_w, req_h, orig_w, orig_h);
                        hr = MFSetAttributeSize(media_type.get(), MF_MT_FRAME_SIZE, orig_w, orig_h);
                        if (SUCCEEDED(hr)) {
                            hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
                            if (SUCCEEDED(hr)) {
                                hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type.get());
                            }
                            if (FAILED(hr)) {
                                hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
                                if (SUCCEEDED(hr)) {
                                    hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type.get());
                                }
                            }
                            if (FAILED(hr)) {
                                hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
                                if (SUCCEEDED(hr)) {
                                    hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type.get());
                                }
                                if (SUCCEEDED(hr)) {
                                    m_output_format = OutputFormat::NV12;
                                    Logger::info("[core] [VideoDecoder] Using NV12 output (original resolution), will convert to BGRA via Video Processor");
                                }
                            }
                            if (SUCCEEDED(hr)) {
                                succeeded_via_retry = true;
                            }
                        }
                    }
                    if (FAILED(hr)) {
                        UINT32 err_w = 0, err_h = 0;
                        MFGetAttributeSize(media_type.get(), MF_MT_FRAME_SIZE, &err_w, &err_h);
                        Logger::error("[core] [VideoDecoder] Failed to set media type (tried ARGB32, RGB32, NV12) at resolution {}x{}, hr = {:#x} (MF_E_INVALIDMEDIATYPE)",
                            err_w, err_h, (uint32_t)hr);
                        Logger::error("[core] [VideoDecoder] This may be due to unsupported output resolution or codec format");
                        return false;
                    }
                } else if (!succeeded_via_retry) {
                    m_output_format = OutputFormat::NV12;
                    Logger::info("[core] [VideoDecoder] Using NV12 output, will convert to BGRA via Video Processor");
                }
            }
        }
        
        hr = m_source_reader->GetCurrentMediaType((DWORD)m_video_stream_index, m_media_type.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to get current media type, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        GUID subtype = GUID_NULL;
        hr = m_media_type->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr)) {
            if (subtype == MFVideoFormat_ARGB32) {
                Logger::info("[core] [VideoDecoder] Using video format: ARGB32 (native BGRA)");
            } else if (subtype == MFVideoFormat_RGB32) {
                Logger::info("[core] [VideoDecoder] Using video format: RGB32");
            } else if (subtype == MFVideoFormat_NV12) {
                Logger::info("[core] [VideoDecoder] Using video format: NV12 (hardware native, GPU conversion to BGRA)");
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
        m_target_size = Vector2U{
            options.output_width > 0 ? options.output_width : width,
            options.output_height > 0 ? options.output_height : height
        };
        
        PROPVARIANT var;
        PropVariantInit(&var);
        hr = m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
        if (SUCCEEDED(hr)) {
            m_duration = var.hVal.QuadPart / 10000000.0;
            PropVariantClear(&var);
        }
        
        UINT32 rate_num = 0, rate_den = 0;
        for (DWORD i = 0; ; ++i) {
            win32::com_ptr<IMFMediaType> mt;
            hr = m_source_reader->GetNativeMediaType((DWORD)m_video_stream_index, i, mt.put());
            if (FAILED(hr)) break;
            if (SUCCEEDED(MFGetAttributeRatio(mt.get(), MF_MT_FRAME_RATE, &rate_num, &rate_den)) && rate_den > 0 && rate_num > 0)
                break;
        }
        if (rate_den == 0 || rate_num == 0) {
            hr = MFGetAttributeRatio(m_media_type.get(), MF_MT_FRAME_RATE, &rate_num, &rate_den);
        }
        if (rate_den > 0 && rate_num > 0) {
            m_frame_interval = static_cast<double>(rate_den) / static_cast<double>(rate_num);
            m_frame_rate_num = rate_num;
            m_frame_rate_den = rate_den;
            Logger::info("[core] [VideoDecoder] Frame rate: {} / {} (= {:.2f} fps)", rate_num, rate_den, 1.0 / m_frame_interval);
        } else {
            m_frame_rate_num = 30;
            m_frame_rate_den = 1;
            m_frame_interval = 1.0 / 30.0;
            Logger::warn("[core] [VideoDecoder] MF_MT_FRAME_RATE not available, assuming 30 fps");
        }
        
        if (!createTexture()) {
            close();
            return false;
        }
        
        if (m_output_format == OutputFormat::NV12 && !createVideoProcessor()) {
            Logger::error("[core] [VideoDecoder] Failed to create Video Processor for NV12 conversion");
            close();
            return false;
        }
        
        m_frame_pitch = m_video_size.x * 4;
        
        m_current_time = 0.0;
        m_last_requested_time = -1.0;
        
        bool first_frame_loaded = readFrameAtTime(0.0);
        Logger::info("[core] [VideoDecoder] First frame loaded: {}", first_frame_loaded);
        
        Logger::info("[core] [VideoDecoder] Opened video: {}x{}, duration: {:.2f}s", 
                    m_target_size.x, m_target_size.y, m_duration);

        m_last_open_path.assign(path.data(), path.size());
        m_last_open_options = options;

        setLooping(options.looping);
        if (options.loop_duration > 0.0) {
            setLoopRange(options.loop_end, options.loop_duration);
        }

        return true;
    }

    void VideoDecoder::getVideoStreams(void (*callback)(VideoStreamInfo const&, void*), void* userdata) const {
        if (!m_source_reader || !callback) return;
        PROPVARIANT var;
        PropVariantInit(&var);
        double duration_sec = 0.0;
        if (SUCCEEDED(m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))) {
            duration_sec = var.hVal.QuadPart / 10000000.0;
            PropVariantClear(&var);
        }
        for (DWORD si = 0; si < kMaxStreams; ++si) {
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
            if (SUCCEEDED(MFGetAttributeRatio(mt.get(), MF_MT_FRAME_RATE, &num, &den)) && den > 0)
                info.fps = static_cast<double>(num) / static_cast<double>(den);
            callback(info, userdata);
        }
    }

    void VideoDecoder::getAudioStreams(void (*callback)(AudioStreamInfo const&, void*), void* userdata) const {
        if (!m_source_reader || !callback) return;
        PROPVARIANT var;
        PropVariantInit(&var);
        double duration_sec = 0.0;
        if (SUCCEEDED(m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))) {
            duration_sec = var.hVal.QuadPart / 10000000.0;
            PropVariantClear(&var);
        }
        for (DWORD si = 0; si < kMaxStreams; ++si) {
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
        m_frame_interval = 1.0 / 30.0;
        m_frame_rate_num = 30;
        m_frame_rate_den = 1;
        m_frame_pitch = 0;
        m_video_stream_index = 0;
        m_has_loop_range = false;
        m_loop_start = 0.0;
        m_loop_end = 0.0;
    }

    void VideoDecoder::setLoopRange(double end_sec, double duration_sec) {
        if (duration_sec > 0.0 && end_sec >= duration_sec) {
            m_loop_start = end_sec - duration_sec;
            double cap = m_duration > 0.0 ? m_duration : end_sec;
            m_loop_end = (std::min)(end_sec, cap);
            m_has_loop_range = (m_loop_end > m_loop_start);
            if (!m_has_loop_range) { m_loop_start = 0.0; m_loop_end = m_duration; }
        } else {
            m_has_loop_range = false;
            m_loop_start = 0.0;
            m_loop_end = m_duration;
        }
    }

    void VideoDecoder::getLoopRange(double* end_sec, double* duration_sec) const noexcept {
        if (end_sec) *end_sec = m_has_loop_range ? m_loop_end : m_duration;
        if (duration_sec) *duration_sec = m_has_loop_range ? (m_loop_end - m_loop_start) : m_duration;
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
        
        auto const loop_begin = m_has_loop_range ? m_loop_start : 0.0;
        auto const loop_end = m_has_loop_range ? m_loop_end : m_duration;
        auto const loop_len = loop_end - loop_begin;

        if (time_in_seconds >= m_duration) {
            if (m_looping) {
                if (loop_end > m_duration && time_in_seconds < loop_end) {
                    constexpr double kEndEpsilon = 1e-6;
                    if (m_current_time < m_duration - kEndEpsilon) {
                        double const last_frame_time = std::max(0.0, m_duration - kEndEpsilon);
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
                constexpr double kEndEpsilon = 1e-6;
                if (m_current_time >= m_duration - kEndEpsilon) {
                    return true;
                }
                double const last_frame_time = std::max(0.0, m_duration - kEndEpsilon);
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
        
        constexpr double kTimeEpsilon = 1e-4;
        constexpr double kBackwardTolerance = 1.0 / 120.0;
        double const frame_tolerance = m_frame_interval * 0.5;
        double const seek_threshold = (std::max)(0.1, 16.0 * m_frame_interval);
        int const max_catch_up_frames = (std::min)(16, (std::max)(1, static_cast<int>(0.2 / m_frame_interval)));

        bool const is_backward = (m_last_requested_time >= 0.0) && (time_in_seconds + kBackwardTolerance < m_last_requested_time);
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
            (DWORD)m_video_stream_index,
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

        for (int i = 0; i < kMaxDecodeFramesAfterSeek; ++i) {
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
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to get ID3D11VideoDevice, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        hr = m_device_context->QueryInterface(IID_PPV_ARGS(m_video_context.put()));
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to get ID3D11VideoContext, hr = {:#x}", (uint32_t)hr);
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
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create VideoProcessorEnumerator, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        hr = m_video_device->CreateVideoProcessor(m_video_processor_enum.get(), 0, m_video_processor.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create VideoProcessor, hr = {:#x}", (uint32_t)hr);
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
        if (FAILED(hr) || buffer_count == 0) {
            return false;
        }
        
        win32::com_ptr<IMFMediaBuffer> buffer;
        hr = sample->GetBufferByIndex(0, buffer.put());
        if (FAILED(hr)) {
            return false;
        }
        
        win32::com_ptr<IMFDXGIBuffer> dxgi_buffer;
        if (FAILED(buffer->QueryInterface(IID_PPV_ARGS(dxgi_buffer.put())))) {
            Logger::error("[core] [VideoDecoder] NV12 sample does not have DXGI buffer");
            return false;
        }
        
        win32::com_ptr<ID3D11Texture2D> nv12_texture;
        hr = dxgi_buffer->GetResource(IID_PPV_ARGS(nv12_texture.put()));
        if (FAILED(hr) || !nv12_texture) {
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
        if (FAILED(hr)) {
            input_view_desc.Texture2D.ArraySlice = 0;
            input_view_desc.Texture2D.MipSlice = 0;
            hr = m_video_device->CreateVideoProcessorInputView(
                nv12_texture.get(), m_video_processor_enum.get(), &input_view_desc, input_view.put());
        }
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create VideoProcessorInputView, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc = {};
        output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
        output_view_desc.Texture2D.MipSlice = 0;
        
        win32::com_ptr<ID3D11VideoProcessorOutputView> output_view;
        hr = m_video_device->CreateVideoProcessorOutputView(
            output_texture, m_video_processor_enum.get(), &output_view_desc, output_view.put());
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] Failed to create VideoProcessorOutputView, hr = {:#x}", (uint32_t)hr);
            return false;
        }
        
        D3D11_VIDEO_PROCESSOR_STREAM stream = {};
        stream.Enable = TRUE;
        stream.OutputIndex = 0;
        stream.InputFrameOrField = 0;
        stream.PastFrames = 0;
        stream.FutureFrames = 0;
        stream.pInputSurface = input_view.get();
        
        hr = m_video_context->VideoProcessorBlt(m_video_processor.get(), output_view.get(), 0, 1, &stream);
        if (FAILED(hr)) {
            Logger::error("[core] [VideoDecoder] VideoProcessorBlt failed, hr = {:#x}", (uint32_t)hr);
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
        hr = sample->ConvertToContiguousBuffer(buffer.put());
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
        
        thread_local bool first_copy = true;
        if (first_copy) {
            Logger::info("[core] [VideoDecoder] Texture update: source={}x{} pitch={}, dest={}x{} pitch={}, force_alpha={}, scale={}",
                        src_w, src_h, source_pitch, dst_w, dst_h, mapped.RowPitch, force_opaque_alpha, need_scale);
            first_copy = false;
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
