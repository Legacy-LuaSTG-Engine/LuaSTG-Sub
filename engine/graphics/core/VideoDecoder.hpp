#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Vector2.hpp"
#include "core/ImmutableString.hpp"

namespace core {
    CORE_INTERFACE IVideoDecoder : IReferenceCounted {
        // 打开视频文件
        virtual bool open(StringView path) = 0;
        
        // 关闭视频
        virtual void close() = 0;
        
        // 获取状态
        virtual bool hasVideo() const noexcept = 0;
        
        // 视频信息
        virtual Vector2U getVideoSize() const noexcept = 0;
        virtual double getDuration() const noexcept = 0;
        virtual double getCurrentTime() const noexcept = 0;  // 返回上次更新的帧时间
        
        // 跳转到指定时间
        virtual bool seek(double time_in_seconds) = 0;
        
        // 循环播放设置
        virtual void setLooping(bool loop) = 0;
        virtual bool isLooping() const noexcept = 0;
        
        // 手动更新到指定时间点
        // time_in_seconds: 目标时间（秒）
        // 返回是否成功更新（失败可能是因为到达视频结尾且未循环）
        virtual bool updateToTime(double time_in_seconds) = 0;
        
        // 获取用于渲染的纹理（返回 ID3D11Texture2D*）
        virtual void* getNativeTexture() const noexcept = 0;
        
        // 获取 Shader Resource View（返回 ID3D11ShaderResourceView*）
        virtual void* getNativeShaderResourceView() const noexcept = 0;
    };

    CORE_INTERFACE_ID(IVideoDecoder, "a4b5c6d7-e8f9-1234-5678-9abcdef01234")
}
