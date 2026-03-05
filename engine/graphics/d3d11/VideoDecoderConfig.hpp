#pragma once
#include <cstdint>

namespace core {
    // Configuration constants for VideoDecoder
    // Using struct with static constexpr for better encapsulation and clarity
    struct VideoDecoderConfig {
        VideoDecoderConfig() = delete;  // Prevent instantiation
        
        // Maximum number of streams to enumerate (video/audio)
        static constexpr uint32_t kMaxStreams = 16;
        
        // Maximum number of frames to decode after seeking to reach the requested timestamp
        static constexpr int kMaxDecodeFramesAfterSeek = 360;
        
        // Time comparison epsilon for frame timing
        static constexpr double kTimeEpsilon = 1e-4;
        
        // Tolerance for detecting backward seeks
        static constexpr double kBackwardTolerance = 1.0 / 120.0;
        
        // Epsilon for detecting end of video
        static constexpr double kEndEpsilon = 1e-6;
        
        // Default frame rate if not available from media
        static constexpr uint32_t kDefaultFrameRateNum = 30;
        static constexpr uint32_t kDefaultFrameRateDen = 1;
    };
}
