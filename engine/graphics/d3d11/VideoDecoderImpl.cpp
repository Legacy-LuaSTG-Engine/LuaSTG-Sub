// VideoDecoder implementation - helper functions
// This file contains the refactored open() function split into logical parts

#include "d3d11/VideoDecoder.hpp"
#include "d3d11/VideoDecoderConfig.hpp"
#include "core/Configuration.hpp"
#include "core/FileSystem.hpp"
#include "core/Logger.hpp"
#include "utf8.hpp"
#include <wil/resource.h>
#include <shlwapi.h>

namespace core {
    using std::string_view_literals::operator ""sv;
    using Config = VideoDecoderConfig;

    bool VideoDecoder::loadVideoFile(StringView path) {
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
        if (!win32::check_hresult_as_boolean(
            MFCreateMFByteStreamOnStream(mem_stream.get(), byte_stream.put()),
            "MFCreateMFByteStreamOnStream"sv
        )) {
            return false;
        }
        
        m_byte_stream = std::move(byte_stream);
        return true;
    }

    bool VideoDecoder::configureHardwareAcceleration() {
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
        HRESULT hr = d3d_device->QueryInterface(IID_PPV_ARGS(video_device.put()));
        if (win32::check_hresult_as_boolean(hr, "ID3D11Device::QueryInterface(ID3D11VideoDevice)"sv)) {
            Logger::info("[core] [VideoDecoder] D3D11 device supports video decoding");
        } else {
            Logger::warn("[core] [VideoDecoder] D3D11 device does not support video decoding");
        }
        
        win32::com_ptr<IMFAttributes> attributes;
        if (!win32::check_hresult_as_boolean(
            MFCreateAttributes(attributes.put(), 3),
            "MFCreateAttributes"sv
        )) {
            return false;
        }
        
        win32::com_ptr<IMFDXGIDeviceManager> dxgi_device_manager;
        UINT dxgi_reset_token = 0;
        bool use_hardware_accel = false;
        
        bool const allow_hw_decode = core::ConfigurationLoader::getInstance().getGraphicsSystem().isAllowHardwareVideoDecode();
        hr = MFCreateDXGIDeviceManager(&dxgi_reset_token, dxgi_device_manager.put());
        if (win32::check_hresult_as_boolean(hr, "MFCreateDXGIDeviceManager"sv)) {
            hr = dxgi_device_manager->ResetDevice(d3d_device, dxgi_reset_token);
            if (win32::check_hresult_as_boolean(hr, "IMFDXGIDeviceManager::ResetDevice"sv)) {
                if (video_device && allow_hw_decode) {
                    hr = attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxgi_device_manager.get());
                    if (win32::check_hresult_as_boolean(hr, "IMFAttributes::SetUnknown(MF_SOURCE_READER_D3D_MANAGER)"sv)) {
                        use_hardware_accel = true;
                        Logger::info("[core] [VideoDecoder] Attempting D3D11 hardware acceleration (IMFDXGIDeviceManager)");
                    }
                } else if (!allow_hw_decode) {
                    Logger::info("[core] [VideoDecoder] Hardware video decode disabled by config");
                } else {
                    Logger::warn("[core] [VideoDecoder] Skipping hardware acceleration - device does not support video");
                }
            }
        }
        
        if (!use_hardware_accel) {
            win32::check_hresult(
                attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE),
                "IMFAttributes::SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS)"sv
            );
            
            win32::check_hresult(
                attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE),
                "IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING)"sv
            );
        }
        
        hr = MFCreateSourceReaderFromByteStream(m_byte_stream.get(), attributes.get(), m_source_reader.put());
        if (!win32::check_hresult_as_boolean(hr, "MFCreateSourceReaderFromByteStream"sv)) {
            if (use_hardware_accel && hr == E_INVALIDARG) {
                Logger::warn("[core] [VideoDecoder] Hardware acceleration failed, retrying without D3D manager");
                attributes->DeleteItem(MF_SOURCE_READER_D3D_MANAGER);
                dxgi_device_manager.reset();
                use_hardware_accel = false;
                
                hr = MFCreateSourceReaderFromByteStream(m_byte_stream.get(), attributes.get(), m_source_reader.put());
            }
            
            if (!win32::check_hresult_as_boolean(hr, "MFCreateSourceReaderFromByteStream (retry)"sv)) {
                return false;
            } else {
                Logger::info("[core] [VideoDecoder] Falling back to software decoding");
            }
        } else if (use_hardware_accel) {
            Logger::info("[core] [VideoDecoder] Using D3D11 hardware acceleration (IMFDXGIDeviceManager)");
        }
        
        return true;
    }

    bool VideoDecoder::selectVideoStream(uint32_t preferred_index) {
        m_video_stream_index = preferred_index;
        if (m_video_stream_index == static_cast<uint32_t>(-1)) {
            for (DWORD i = 0; i < Config::kMaxStreams; ++i) {
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
        
        for (DWORD i = 0; i < Config::kMaxStreams; ++i) {
            BOOL const select = (i == m_video_stream_index);
            m_source_reader->SetStreamSelection(i, select);
        }
        
        return true;
    }

    bool VideoDecoder::negotiateOutputFormat(VideoOpenOptions const& options) {
        win32::com_ptr<IMFMediaType> partial_media_type;
        HRESULT hr = m_source_reader->GetNativeMediaType((DWORD)m_video_stream_index, 0, partial_media_type.put());
        if (!win32::check_hresult_as_boolean(hr, "IMFSourceReader::GetNativeMediaType"sv)) {
            Logger::warn("[core] [VideoDecoder] Failed to get native media type, trying current type");
            hr = m_source_reader->GetCurrentMediaType((DWORD)m_video_stream_index, partial_media_type.put());
            win32::check_hresult(hr, "IMFSourceReader::GetCurrentMediaType"sv);
        }
        
        logNativeFormat(partial_media_type.get());
        
        win32::com_ptr<IMFMediaType> media_type;
        if (!win32::check_hresult_as_boolean(
            MFCreateMediaType(media_type.put()),
            "MFCreateMediaType"sv
        )) {
            return false;
        }
        
        if (!win32::check_hresult_as_boolean(
            media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video),
            "IMFMediaType::SetGUID(MF_MT_MAJOR_TYPE)"sv
        )) {
            return false;
        }
        
        enumerateOutputFormats(partial_media_type.get());
        
        if (!configureOutputResolution(partial_media_type.get(), media_type.get(), options)) {
            return false;
        }
        
        if (!trySetOutputFormat(partial_media_type.get(), media_type.get())) {
            return false;
        }
        
        if (!win32::check_hresult_as_boolean(
            m_source_reader->GetCurrentMediaType((DWORD)m_video_stream_index, m_media_type.put()),
            "IMFSourceReader::GetCurrentMediaType"sv
        )) {
            return false;
        }
        
        logFinalFormat();
        
        return true;
    }

    void VideoDecoder::logNativeFormat(IMFMediaType* partial_media_type) {
        if (!partial_media_type) return;
        
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

    void VideoDecoder::enumerateOutputFormats(IMFMediaType* partial_media_type) {
        if (!partial_media_type) return;
        
        GUID native_subtype = GUID_NULL;
        if (SUCCEEDED(partial_media_type->GetGUID(MF_MT_SUBTYPE, &native_subtype))) {
            if (native_subtype == MFVideoFormat_H264) {
                Logger::info("[core] [VideoDecoder] Hardware decoder outputs compressed format, enumerating supported output formats");
                for (DWORD i = 0; ; ++i) {
                    win32::com_ptr<IMFMediaType> output_type;
                    if (!win32::check_hresult_as_boolean(
                        m_source_reader->GetNativeMediaType((DWORD)m_video_stream_index, i, output_type.put()),
                        "IMFSourceReader::GetNativeMediaType"sv
                    )) {
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

    bool VideoDecoder::configureOutputResolution(IMFMediaType* partial_media_type, IMFMediaType* media_type, VideoOpenOptions const& options) {
        if (!partial_media_type) return true;
        
        UINT32 width = 0, height = 0;
        if (SUCCEEDED(MFGetAttributeSize(partial_media_type, MF_MT_FRAME_SIZE, &width, &height))) {
            UINT32 out_w = options.output_width > 0 ? options.output_width : width;
            UINT32 out_h = options.output_height > 0 ? options.output_height : height;
            if (out_w != width || out_h != height) {
                Logger::info("[core] [VideoDecoder] Requesting output resolution: {}x{} (original: {}x{})", out_w, out_h, width, height);
            }
            HRESULT hr = MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, out_w, out_h);
            if (!win32::check_hresult_as_boolean(hr, "MFSetAttributeSize"sv)) {
                Logger::warn("[core] [VideoDecoder] Failed to set output resolution {}x{}, using original {}x{}", 
                    out_w, out_h, width, height);
                win32::check_hresult(
                    MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, width, height),
                    "MFSetAttributeSize (original)"sv
                );
            }
        }
        
        MFVideoArea area = {};
        if (SUCCEEDED(partial_media_type->GetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(area), nullptr))) {
            media_type->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(area));
        }
        
        return true;
    }

    bool VideoDecoder::trySetOutputFormat(IMFMediaType* partial_media_type, IMFMediaType* media_type) {
        if (!win32::check_hresult_as_boolean(
            media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32),
            "IMFMediaType::SetGUID(MFVideoFormat_ARGB32)"sv
        )) {
            return false;
        }
        
        HRESULT hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type);
        
        if (!win32::check_hresult_as_boolean(hr, "IMFSourceReader::SetCurrentMediaType(ARGB32)"sv)) {
            Logger::info("[core] [VideoDecoder] ARGB32 not supported, trying RGB32");
            if (!win32::check_hresult_as_boolean(
                media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32),
                "IMFMediaType::SetGUID(MFVideoFormat_RGB32)"sv
            )) {
                return false;
            }
            hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type);
            
            if (!win32::check_hresult_as_boolean(hr, "IMFSourceReader::SetCurrentMediaType(RGB32)"sv)) {
                return tryNV12Format(partial_media_type, media_type);
            }
        }
        
        return true;
    }

    bool VideoDecoder::tryNV12Format(IMFMediaType* partial_media_type, IMFMediaType* media_type) {
        Logger::info("[core] [VideoDecoder] RGB32 not supported, trying NV12 (hardware native)");
        if (!win32::check_hresult_as_boolean(
            media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12),
            "IMFMediaType::SetGUID(MFVideoFormat_NV12)"sv
        )) {
            return false;
        }
        
        HRESULT hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type);
        bool succeeded_via_retry = false;
        
        if (!win32::check_hresult_as_boolean(hr, "IMFSourceReader::SetCurrentMediaType(NV12)"sv)) {
            if (!retryWithOriginalResolution(partial_media_type, media_type, hr, succeeded_via_retry)) {
                return false;
            }
        } else if (!succeeded_via_retry) {
            m_output_format = OutputFormat::NV12;
            Logger::info("[core] [VideoDecoder] Using NV12 output, will convert to BGRA via Video Processor");
        }
        
        return true;
    }

    bool VideoDecoder::retryWithOriginalResolution(IMFMediaType* partial_media_type, IMFMediaType* media_type, HRESULT& hr, bool& succeeded) {
        UINT32 req_w = 0, req_h = 0;
        MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &req_w, &req_h);
        UINT32 orig_w = 0, orig_h = 0;
        bool can_retry_original = partial_media_type
            && SUCCEEDED(MFGetAttributeSize(partial_media_type, MF_MT_FRAME_SIZE, &orig_w, &orig_h))
            && (req_w != orig_w || req_h != orig_h);
            
        if (can_retry_original) {
            Logger::info("[core] [VideoDecoder] Scaled resolution {}x{} not supported by decoder, retrying with original {}x{}",
                req_w, req_h, orig_w, orig_h);
            hr = MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, orig_w, orig_h);
            if (SUCCEEDED(hr)) {
                hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
                if (SUCCEEDED(hr)) {
                    hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type);
                }
                if (!win32::check_hresult_as_boolean(hr, "SetCurrentMediaType(ARGB32, original)"sv)) {
                    hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
                    if (SUCCEEDED(hr)) {
                        hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type);
                    }
                }
                if (!win32::check_hresult_as_boolean(hr, "SetCurrentMediaType(RGB32, original)"sv)) {
                    hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
                    if (SUCCEEDED(hr)) {
                        hr = m_source_reader->SetCurrentMediaType((DWORD)m_video_stream_index, nullptr, media_type);
                    }
                    if (win32::check_hresult_as_boolean(hr, "SetCurrentMediaType(NV12, original)"sv)) {
                        m_output_format = OutputFormat::NV12;
                        Logger::info("[core] [VideoDecoder] Using NV12 output (original resolution), will convert to BGRA via Video Processor");
                    }
                }
                if (win32::check_hresult_as_boolean(hr, "SetCurrentMediaType (retry)"sv)) {
                    succeeded = true;
                }
            }
        }
        
        if (!win32::check_hresult_as_boolean(hr, "SetCurrentMediaType (final)"sv)) {
            UINT32 err_w = 0, err_h = 0;
            MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, &err_w, &err_h);
            Logger::error("[core] [VideoDecoder] Failed to set media type (tried ARGB32, RGB32, NV12) at resolution {}x{}",
                err_w, err_h);
            Logger::error("[core] [VideoDecoder] This may be due to unsupported output resolution or codec format");
            return false;
        }
        
        return true;
    }

    void VideoDecoder::logFinalFormat() {
        GUID subtype = GUID_NULL;
        HRESULT hr = m_media_type->GetGUID(MF_MT_SUBTYPE, &subtype);
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
    }

    bool VideoDecoder::extractVideoProperties() {
        UINT32 width = 0, height = 0;
        if (!win32::check_hresult_as_boolean(
            MFGetAttributeSize(m_media_type.get(), MF_MT_FRAME_SIZE, &width, &height),
            "MFGetAttributeSize(MF_MT_FRAME_SIZE)"sv
        )) {
            return false;
        }
        
        m_video_size = Vector2U{ width, height };
        m_target_size = m_video_size;
        
        wil::unique_prop_variant var;
        if (SUCCEEDED(m_source_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))) {
            m_duration = var.hVal.QuadPart / 10000000.0;
        }
        
        UINT32 rate_num = 0, rate_den = 0;
        for (DWORD i = 0; ; ++i) {
            win32::com_ptr<IMFMediaType> mt;
            if (!win32::check_hresult_as_boolean(
                m_source_reader->GetNativeMediaType((DWORD)m_video_stream_index, i, mt.put()),
                "IMFSourceReader::GetNativeMediaType"sv
            )) {
                break;
            }
            if (SUCCEEDED(MFGetAttributeRatio(mt.get(), MF_MT_FRAME_RATE, &rate_num, &rate_den)) && rate_den > 0 && rate_num > 0)
                break;
        }
        if (rate_den == 0 || rate_num == 0) {
            HRESULT hr = MFGetAttributeRatio(m_media_type.get(), MF_MT_FRAME_RATE, &rate_num, &rate_den);
        }
        if (rate_den > 0 && rate_num > 0) {
            m_frame_interval = static_cast<double>(rate_den) / static_cast<double>(rate_num);
            m_frame_rate_num = rate_num;
            m_frame_rate_den = rate_den;
            Logger::info("[core] [VideoDecoder] Frame rate: {} / {} (= {:.2f} fps)", rate_num, rate_den, 1.0 / m_frame_interval);
        } else {
            m_frame_rate_num = Config::kDefaultFrameRateNum;
            m_frame_rate_den = Config::kDefaultFrameRateDen;
            m_frame_interval = 1.0 / Config::kDefaultFrameRateNum;
            Logger::warn("[core] [VideoDecoder] MF_MT_FRAME_RATE not available, assuming 30 fps");
        }
        
        return true;
    }

    bool VideoDecoder::createResources() {
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
        return true;
    }

    bool VideoDecoder::loadFirstFrame() {
        m_current_time = 0.0;
        m_last_requested_time = -1.0;
        
        bool first_frame_loaded = readFrameAtTime(0.0);
        Logger::info("[core] [VideoDecoder] First frame loaded: {}", first_frame_loaded);
        return true;
    }

} // namespace core
