#pragma once
#include "core/VideoDecoder.hpp"
#include "core/Texture2D.hpp"
#include "lua/plus.hpp"
#include <vector>

namespace luastg::binding::video {
    // Parse VideoOpenOptions from Lua table
    inline void parseVideoOptions(lua_State* L, int index, core::VideoOpenOptions& opt) noexcept {
        lua::stack_t stack(L);
        lua::stack_index_t table_idx(index);
        
        if (!stack.is_table(table_idx)) {
            return;
        }
        
        opt.video_stream_index = stack.get_map_value<uint32_t>(table_idx, "video_stream", opt.video_stream_index);
        opt.output_width = stack.get_map_value<uint32_t>(table_idx, "width", opt.output_width);
        opt.output_height = stack.get_map_value<uint32_t>(table_idx, "height", opt.output_height);
        opt.premultiplied_alpha = stack.get_map_value<bool>(table_idx, "premultiplied_alpha", opt.premultiplied_alpha);
        opt.looping = stack.get_map_value<bool>(table_idx, "looping", opt.looping);
        opt.loop_end = stack.get_map_value<double>(table_idx, "loop_end", opt.loop_end);
        opt.loop_duration = stack.get_map_value<double>(table_idx, "loop_duration", opt.loop_duration);
    }

    // Get video decoder from texture object
    inline core::IVideoDecoder* getDecoderFromTexture(core::ITexture2D* texture) noexcept {
        if (!texture || !texture->isVideoTexture()) {
            return nullptr;
        }
        return texture->getVideoDecoder();
    }

    // Get video decoder from resource name (via resource manager)
    core::IVideoDecoder* getDecoderFromResourceName(const char* name) noexcept;

    // Push video stream info array to Lua stack
    inline void pushVideoStreamsToLua(lua_State* L, core::IVideoDecoder* decoder) {
        lua::stack_t stack(L);
        
        if (!decoder) {
            stack.create_array(0);
            return;
        }

        std::vector<core::VideoStreamInfo> list;
        auto callback = [](core::VideoStreamInfo const& info, void* userdata) {
            static_cast<std::vector<core::VideoStreamInfo>*>(userdata)->push_back(info);
        };
        decoder->getVideoStreams(callback, &list);

        auto array_idx = stack.create_array(list.size());
        for (size_t i = 0; i < list.size(); ++i) {
            auto item_idx = stack.create_map(5);
            stack.set_map_value(item_idx, "index", static_cast<uint32_t>(list[i].index));
            stack.set_map_value(item_idx, "width", static_cast<uint32_t>(list[i].width));
            stack.set_map_value(item_idx, "height", static_cast<uint32_t>(list[i].height));
            stack.set_map_value(item_idx, "fps", list[i].fps);
            stack.set_map_value(item_idx, "duration", list[i].duration_seconds);
            stack.set_array_value(array_idx, lua::stack_index_t(static_cast<int32_t>(i + 1)), item_idx);
        }
    }

    // Push audio stream info array to Lua stack
    inline void pushAudioStreamsToLua(lua_State* L, core::IVideoDecoder* decoder) {
        lua::stack_t stack(L);
        
        if (!decoder) {
            stack.create_array(0);
            return;
        }

        std::vector<core::AudioStreamInfo> list;
        auto callback = [](core::AudioStreamInfo const& info, void* userdata) {
            static_cast<std::vector<core::AudioStreamInfo>*>(userdata)->push_back(info);
        };
        decoder->getAudioStreams(callback, &list);

        auto array_idx = stack.create_array(list.size());
        for (size_t i = 0; i < list.size(); ++i) {
            auto item_idx = stack.create_map(4);
            stack.set_map_value(item_idx, "index", static_cast<uint32_t>(list[i].index));
            stack.set_map_value(item_idx, "channels", static_cast<uint32_t>(list[i].channels));
            stack.set_map_value(item_idx, "sample_rate", static_cast<uint32_t>(list[i].sample_rate));
            stack.set_map_value(item_idx, "duration", list[i].duration_seconds);
            stack.set_array_value(array_idx, lua::stack_index_t(static_cast<int32_t>(i + 1)), item_idx);
        }
    }

    // Push video info table to Lua stack
    inline void pushVideoInfoToLua(lua_State* L, core::IVideoDecoder* decoder) {
        lua::stack_t stack(L);
        
        if (!decoder) {
            stack.create_map(0);
            return;
        }

        auto map_idx = stack.create_map(10);
        
        stack.set_map_value(map_idx, "duration", decoder->getDuration());
        stack.set_map_value(map_idx, "time", decoder->getCurrentTime());
        stack.set_map_value(map_idx, "looping", decoder->isLooping());

        double loop_end = 0.0, loop_duration = 0.0;
        decoder->getLoopRange(&loop_end, &loop_duration);
        stack.set_map_value(map_idx, "loop_end", loop_end);
        stack.set_map_value(map_idx, "loop_duration", loop_duration);
        
        auto size = decoder->getVideoSize();
        stack.set_map_value(map_idx, "width", static_cast<int32_t>(size.x));
        stack.set_map_value(map_idx, "height", static_cast<int32_t>(size.y));
        
        stack.set_map_value(map_idx, "video_stream", static_cast<uint32_t>(decoder->getVideoStreamIndex()));
        
        double frame_interval = decoder->getFrameInterval();
        stack.set_map_value(map_idx, "frame_interval", frame_interval);
        
        if (frame_interval > 0.0) {
            stack.set_map_value(map_idx, "fps", 1.0 / frame_interval);
        }
    }
}
