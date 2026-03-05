#pragma once
#include "core/VideoDecoder.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/VideoDecoderConfig.hpp"
#include "d3d11/pch.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mutex>
#include <string>

namespace core {
    class VideoDecoder final :
        public implement::ReferenceCounted<IVideoDecoder>,
        public IGraphicsDeviceEventListener {
    private:
        enum class OutputFormat {
            ARGB32,
            RGB32,
            NV12
        };

        struct LoopState {
            bool has_range{ false };
            double start{ 0.0 };
            double end{ 0.0 };
        };
        
    public:
        // IVideoDecoder

        bool open(StringView path) override;
        bool open(StringView path, VideoOpenOptions const& options) override;
        void close() override;

        bool hasVideo() const noexcept override { return m_source_reader.get() != nullptr; }

        Vector2U getVideoSize() const noexcept override { return m_target_size; }
        double getDuration() const noexcept override { return m_duration; }
        double getCurrentTime() const noexcept override { return m_current_time; }
        double getFrameInterval() const noexcept override { return m_frame_interval; }

        bool seek(double time_in_seconds) override;

        void setLooping(bool loop) override { m_looping = loop; }
        bool isLooping() const noexcept override { return m_looping; }
        void setLoopRange(double end_sec, double duration_sec) override;
        void getLoopRange(double* end_sec, double* duration_sec) const noexcept override;

        bool updateToTime(double time_in_seconds) override;
        void* getNativeTexture() const noexcept override { return m_texture.get(); }
        void* getNativeShaderResourceView() const noexcept override { return m_shader_resource_view.get(); }

        uint32_t getVideoStreamIndex() const noexcept override { return m_video_stream_index; }
        void getVideoStreams(void (*callback)(VideoStreamInfo const&, void*), void* userdata) const override;
        void getAudioStreams(void (*callback)(AudioStreamInfo const&, void*), void* userdata) const override;
        bool reopen(VideoOpenOptions const& options) override;
        VideoOpenOptions getLastOpenOptions() const noexcept override { return m_last_open_options; }
        std::string_view getLastOpenPath() const noexcept override { return m_last_open_path; }

        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        // VideoDecoder

        VideoDecoder();
        VideoDecoder(VideoDecoder const&) = delete;
        VideoDecoder(VideoDecoder&&) = delete;
        ~VideoDecoder();

        VideoDecoder& operator=(VideoDecoder const&) = delete;
        VideoDecoder& operator=(VideoDecoder&&) = delete;

        bool initialize(IGraphicsDevice* device);

    private:
        // File loading
        bool loadVideoFile(StringView path);
        
        // Hardware acceleration setup
        bool configureHardwareAcceleration();
        
        // Stream selection
        bool selectVideoStream(uint32_t preferred_index);
        
        // Output format negotiation
        bool negotiateOutputFormat(VideoOpenOptions const& options);
        void logNativeFormat(IMFMediaType* partial_media_type);
        void enumerateOutputFormats(IMFMediaType* partial_media_type);
        bool configureOutputResolution(IMFMediaType* partial_media_type, IMFMediaType* media_type, VideoOpenOptions const& options);
        bool trySetOutputFormat(IMFMediaType* partial_media_type, IMFMediaType* media_type);
        bool tryNV12Format(IMFMediaType* partial_media_type, IMFMediaType* media_type);
        bool retryWithOriginalResolution(IMFMediaType* partial_media_type, IMFMediaType* media_type, HRESULT& hr, bool& succeeded);
        void logFinalFormat();
        
        // Property extraction
        bool extractVideoProperties();
        
        // Resource creation
        bool createResources();
        bool createTexture();
        bool createVideoProcessor();
        
        // Frame loading
        bool loadFirstFrame();
        
        // Frame decoding
        bool updateTextureFromSample(IMFSample* sample);
        bool updateTextureFromNV12Sample(IMFSample* sample);
        bool updateTextureFromNV12SampleTo(IMFSample* sample, ID3D11Texture2D* output_texture);
        bool readNextFrame();
        bool readFrameAtTime(double time_in_seconds);

        std::mutex m_device_context_mutex;
        SmartReference<IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Texture2D> m_texture;
        win32::com_ptr<ID3D11ShaderResourceView> m_shader_resource_view;
        win32::com_ptr<ID3D11DeviceContext> m_device_context;
        
        win32::com_ptr<IMFSourceReader> m_source_reader;
        win32::com_ptr<IMFByteStream> m_byte_stream;
        win32::com_ptr<IMFMediaType> m_media_type;
        
        win32::com_ptr<ID3D11VideoDevice> m_video_device;
        win32::com_ptr<ID3D11VideoContext> m_video_context;
        win32::com_ptr<ID3D11VideoProcessor> m_video_processor;
        win32::com_ptr<ID3D11VideoProcessorEnumerator> m_video_processor_enum;
        OutputFormat m_output_format{ OutputFormat::ARGB32 };
        Vector2U m_video_size{};
        Vector2U m_target_size{};
        double m_duration{ 0.0 };
        double m_current_time{ 0.0 };
        double m_last_requested_time{ -1.0 };
        double m_frame_interval{ 1.0 / VideoDecoderConfig::kDefaultFrameRateNum };
        uint32_t m_frame_rate_num{ VideoDecoderConfig::kDefaultFrameRateNum };
        uint32_t m_frame_rate_den{ VideoDecoderConfig::kDefaultFrameRateDen };
        uint32_t m_frame_pitch{ 0 };
        uint32_t m_video_stream_index{ 0 };
        bool m_looping{ false };
        LoopState m_loop_state{};
        std::string m_last_open_path;
        VideoOpenOptions m_last_open_options;
        bool m_initialized{ false };
        bool m_first_texture_update_logged{ false };
    };
}
