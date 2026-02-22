#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"
#include "core/ImmutableString.hpp"
#include <cstdint>
#include <string_view>

namespace core {
    // 打开选项：流选择、输出尺寸、预乘 Alpha、循环等
    struct VideoOpenOptions {
        uint32_t video_stream_index{ static_cast<uint32_t>(-1) };  // -1 = 自动选择第一个视频流
        uint32_t output_width{ 0 };   // 0 = 使用视频原始宽度
        uint32_t output_height{ 0 };  // 0 = 使用视频原始高度
        bool premultiplied_alpha{ false };
        bool looping{ false };
        double loop_end{ 0.0 };       // 循环区间结束时间(秒)，仅当 loop_duration > 0 时生效
        double loop_duration{ 0.0 };  // 循环区间长度(秒)，> 0 时与 loop_end 一起设置循环区间 [loop_end-duration, loop_end)
    };

    // 流枚举结果（用于 getVideoStreams / getAudioStreams）
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
        // 打开视频文件（使用默认选项时可直接 open(path)）
        virtual bool open(StringView path) = 0;
        virtual bool open(StringView path, VideoOpenOptions const& options) = 0;
        
        // 关闭视频
        virtual void close() = 0;
        
        // 获取状态
        virtual bool hasVideo() const noexcept = 0;
        
        // 视频信息
        virtual Vector2U getVideoSize() const noexcept = 0;
        virtual double getDuration() const noexcept = 0;
        virtual double getCurrentTime() const noexcept = 0;  // 返回上次更新的帧时间
        virtual double getFrameInterval() const noexcept = 0;  // 每帧时长（秒），用于 fps = 1/getFrameInterval()
        
        // 跳转到指定时间
        virtual bool seek(double time_in_seconds) = 0;
        
        // 循环播放设置
        virtual void setLooping(bool loop) = 0;
        virtual bool isLooping() const noexcept = 0;
        // 区间 [end_sec - duration_sec, end_sec)，超过 end 时减去 duration
        virtual void setLoopRange(double end_sec, double duration_sec) = 0;
        virtual void getLoopRange(double* end_sec, double* duration_sec) const noexcept = 0;
        
        // 手动更新到指定时间点
        // time_in_seconds: 目标时间（秒）
        // 返回是否成功更新（失败可能是因为到达视频结尾且未循环）
        virtual bool updateToTime(double time_in_seconds) = 0;
        
        // 获取用于渲染的纹理（返回 ID3D11Texture2D*）
        virtual void* getNativeTexture() const noexcept = 0;
        
        // 获取 Shader Resource View（返回 ID3D11ShaderResourceView*）
        virtual void* getNativeShaderResourceView() const noexcept = 0;

        // 当前使用的视频流索引
        virtual uint32_t getVideoStreamIndex() const noexcept = 0;

        // 从当前已打开的 SourceReader 枚举流（仅在 hasVideo() 时有效，无需再次读文件）
        virtual void getVideoStreams(void (*callback)(VideoStreamInfo const&, void*), void* userdata) const = 0;
        virtual void getAudioStreams(void (*callback)(AudioStreamInfo const&, void*), void* userdata) const = 0;

        // 使用上次打开的路径重新打开，仅更新 options（用于切换视频流等）
        virtual bool reopen(VideoOpenOptions const& options) = 0;
        virtual VideoOpenOptions getLastOpenOptions() const noexcept = 0;
        virtual std::string_view getLastOpenPath() const noexcept = 0;
    };

    CORE_INTERFACE_ID(IVideoDecoder, "a4b5c6d7-e8f9-1234-5678-9abcdef01234")
}
