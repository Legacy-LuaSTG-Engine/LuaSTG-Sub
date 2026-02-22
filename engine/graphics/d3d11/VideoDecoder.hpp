#pragma once
#include "core/VideoDecoder.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "d3d11/pch.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mutex>

namespace core {
    class VideoDecoder final :
        public implement::ReferenceCounted<IVideoDecoder>,
        public IGraphicsDeviceEventListener {
    public:
        // IVideoDecoder
        
        bool open(StringView path) override;
        void close() override;
        
        bool hasVideo() const noexcept override { return m_source_reader.get() != nullptr; }
        
        Vector2U getVideoSize() const noexcept override { return m_target_size; }
        double getDuration() const noexcept override { return m_duration; }
        double getCurrentTime() const noexcept override { return m_current_time; }
        double getFrameInterval() const noexcept override { return m_frame_interval; }
        
        bool seek(double time_in_seconds) override;
        
        void setLooping(bool loop) override { m_looping = loop; }
        bool isLooping() const noexcept override { return m_looping; }
        
        bool updateToTime(double time_in_seconds) override;
        void* getNativeTexture() const noexcept override { return m_texture.get(); }
        void* getNativeShaderResourceView() const noexcept override { return m_shader_resource_view.get(); }
        
        // IGraphicsDeviceEventListener
        
        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;
        
        // VideoDecoder
        
        VideoDecoder();
        ~VideoDecoder();
        
        bool initialize(IGraphicsDevice* device);
        
    private:
        bool createTexture();
        bool createVideoProcessor();
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
        
        bool m_output_format_nv12{ false };
        
        Vector2U m_video_size{};
        Vector2U m_target_size{};
        double m_duration{ 0.0 };
        double m_current_time{ 0.0 };
        double m_last_requested_time{ -1.0 };
        double m_frame_interval{ 1.0 / 30.0 };
        bool m_looping{ false };
        
        uint32_t m_frame_pitch{ 0 };
        
        bool m_initialized{ false };
    };
}
