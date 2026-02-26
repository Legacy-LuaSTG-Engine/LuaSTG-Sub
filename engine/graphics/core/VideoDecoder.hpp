#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"
#include "core/ImmutableString.hpp"
#include <cstdint>
#include <string_view>

namespace core {
    struct VideoOpenOptions {
        uint32_t video_stream_index{ static_cast<uint32_t>(-1) };
        uint32_t output_width{ 0 };
        uint32_t output_height{ 0 };
        bool premultiplied_alpha{ false };
        bool looping{ false };
        double loop_end{ 0.0 };
        double loop_duration{ 0.0 };
    };

    struct VideoStreamInfo {
        uint32_t index{ 0 };
        uint32_t width{ 0 };
        uint32_t height{ 0 };
        double fps{ 0.0 };
        double duration_seconds{ 0.0 };
    };

    struct AudioStreamInfo {
        uint32_t index{ 0 };
        uint32_t channels{ 0 };
        uint32_t sample_rate{ 0 };
        double duration_seconds{ 0.0 };
    };

    CORE_INTERFACE IVideoDecoder : IReferenceCounted {
        virtual bool open(StringView path) = 0;
        virtual bool open(StringView path, VideoOpenOptions const& options) = 0;

        virtual void close() = 0;

        virtual bool hasVideo() const noexcept = 0;

        virtual Vector2U getVideoSize() const noexcept = 0;
        virtual double getDuration() const noexcept = 0;
        virtual double getCurrentTime() const noexcept = 0;
        virtual double getFrameInterval() const noexcept = 0;

        virtual bool seek(double time_in_seconds) = 0;

        virtual void setLooping(bool loop) = 0;
        virtual bool isLooping() const noexcept = 0;
        virtual void setLoopRange(double end_sec, double duration_sec) = 0;
        virtual void getLoopRange(double* end_sec, double* duration_sec) const noexcept = 0;

        virtual bool updateToTime(double time_in_seconds) = 0;

        virtual void* getNativeTexture() const noexcept = 0;
        virtual void* getNativeShaderResourceView() const noexcept = 0;

        virtual uint32_t getVideoStreamIndex() const noexcept = 0;
        virtual void getVideoStreams(void (*callback)(VideoStreamInfo const&, void*), void* userdata) const = 0;
        virtual void getAudioStreams(void (*callback)(AudioStreamInfo const&, void*), void* userdata) const = 0;

        virtual bool reopen(VideoOpenOptions const& options) = 0;
        virtual VideoOpenOptions getLastOpenOptions() const noexcept = 0;
        virtual std::string_view getLastOpenPath() const noexcept = 0;
    };

    CORE_INTERFACE_ID(IVideoDecoder, "a4b5c6d7-e8f9-1234-5678-9abcdef01234")
}
