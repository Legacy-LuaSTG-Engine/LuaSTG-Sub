#include "VideoBindingHelpers.hpp"
#include "AppFrame.h"

namespace luastg::binding::video {
    void parseVideoOptions(lua_State* const vm, const int index, core::VideoOpenOptions& opt) {
        lua::stack_t stack(vm);
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

    void pushVideoStreamsToLua(lua_State* const vm, core::IVideoDecoder* const decoder) {
        lua::stack_t stack(vm);
        
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
        int index = 1;
        for (auto const& info : list) {
            auto item_idx = stack.create_map(5);
            
            stack.set_map_value(item_idx, "index", info.index);
            stack.set_map_value(item_idx, "width", info.width);
            stack.set_map_value(item_idx, "height", info.height);
            stack.set_map_value(item_idx, "fps", info.fps);
            stack.set_map_value(item_idx, "duration", info.duration_seconds);
            
            stack.set_array_value(array_idx, index, item_idx);
            stack.pop_value();
            ++index;
        }
    }

    void pushAudioStreamsToLua(lua_State* const vm, core::IVideoDecoder* const decoder) {
        lua::stack_t stack(vm);
        
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
        int index = 1;
        for (auto const& info : list) {
            auto item_idx = stack.create_map(4);
            
            stack.set_map_value(item_idx, "index", info.index);
            stack.set_map_value(item_idx, "channels", info.channels);
            stack.set_map_value(item_idx, "sample_rate", info.sample_rate);
            stack.set_map_value(item_idx, "duration", info.duration_seconds);
            
            stack.set_array_value(array_idx, index, item_idx);
            stack.pop_value();
            ++index;
        }
    }

    void pushVideoInfoToLua(lua_State* const vm, core::IVideoDecoder* const decoder) {
        lua::stack_t stack(vm);
        
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
        stack.set_map_value(map_idx, "width", size.x);
        stack.set_map_value(map_idx, "height", size.y);
        
        stack.set_map_value(map_idx, "video_stream", decoder->getVideoStreamIndex());
        
        double frame_interval = decoder->getFrameInterval();
        stack.set_map_value(map_idx, "frame_interval", frame_interval);
        
        if (frame_interval > 0.0) {
            stack.set_map_value(map_idx, "fps", 1.0 / frame_interval);
        }
    }
}
