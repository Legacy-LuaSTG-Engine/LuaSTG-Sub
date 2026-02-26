#pragma once
#include "core/VideoDecoder.hpp"
#include "core/Texture2D.hpp"
#include "lua.hpp"
#include <vector>

namespace luastg::binding::video {
    // Lua table field extraction helpers
    template<typename T>
    inline void getOptionalField(lua_State* L, int table_idx, const char* field, T& out) noexcept {
        lua_getfield(L, table_idx, field);
        if constexpr (std::is_same_v<T, uint32_t>) {
            if (lua_isnumber(L, -1)) {
                out = static_cast<uint32_t>(lua_tointeger(L, -1));
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            if (lua_isboolean(L, -1)) {
                out = (lua_toboolean(L, -1) != 0);
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (lua_isnumber(L, -1)) {
                out = lua_tonumber(L, -1);
            }
        }
        lua_pop(L, 1);
    }

    // Parse VideoOpenOptions from Lua table
    inline void parseVideoOptions(lua_State* L, int index, core::VideoOpenOptions& opt) noexcept {
        if (!lua_istable(L, index)) {
            return;
        }
        
        getOptionalField(L, index, "video_stream", opt.video_stream_index);
        getOptionalField(L, index, "width", opt.output_width);
        getOptionalField(L, index, "height", opt.output_height);
        getOptionalField(L, index, "premultiplied_alpha", opt.premultiplied_alpha);
        getOptionalField(L, index, "looping", opt.looping);
        getOptionalField(L, index, "loop_end", opt.loop_end);
        getOptionalField(L, index, "loop_duration", opt.loop_duration);
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
        if (!decoder) {
            lua_createtable(L, 0, 0);
            return;
        }

        std::vector<core::VideoStreamInfo> list;
        auto callback = [](core::VideoStreamInfo const& info, void* userdata) {
            static_cast<std::vector<core::VideoStreamInfo>*>(userdata)->push_back(info);
        };
        decoder->getVideoStreams(callback, &list);

        lua_createtable(L, static_cast<int>(list.size()), 0);
        for (size_t i = 0; i < list.size(); ++i) {
            lua_createtable(L, 0, 5);
            lua_pushinteger(L, static_cast<lua_Integer>(list[i].index));
            lua_setfield(L, -2, "index");
            lua_pushinteger(L, static_cast<lua_Integer>(list[i].width));
            lua_setfield(L, -2, "width");
            lua_pushinteger(L, static_cast<lua_Integer>(list[i].height));
            lua_setfield(L, -2, "height");
            lua_pushnumber(L, list[i].fps);
            lua_setfield(L, -2, "fps");
            lua_pushnumber(L, list[i].duration_seconds);
            lua_setfield(L, -2, "duration");
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
    }

    // Push audio stream info array to Lua stack
    inline void pushAudioStreamsToLua(lua_State* L, core::IVideoDecoder* decoder) {
        if (!decoder) {
            lua_createtable(L, 0, 0);
            return;
        }

        std::vector<core::AudioStreamInfo> list;
        auto callback = [](core::AudioStreamInfo const& info, void* userdata) {
            static_cast<std::vector<core::AudioStreamInfo>*>(userdata)->push_back(info);
        };
        decoder->getAudioStreams(callback, &list);

        lua_createtable(L, static_cast<int>(list.size()), 0);
        for (size_t i = 0; i < list.size(); ++i) {
            lua_createtable(L, 0, 4);
            lua_pushinteger(L, static_cast<lua_Integer>(list[i].index));
            lua_setfield(L, -2, "index");
            lua_pushinteger(L, static_cast<lua_Integer>(list[i].channels));
            lua_setfield(L, -2, "channels");
            lua_pushinteger(L, static_cast<lua_Integer>(list[i].sample_rate));
            lua_setfield(L, -2, "sample_rate");
            lua_pushnumber(L, list[i].duration_seconds);
            lua_setfield(L, -2, "duration");
            lua_rawseti(L, -2, static_cast<int>(i) + 1);
        }
    }

    // Push video info table to Lua stack
    inline void pushVideoInfoToLua(lua_State* L, core::IVideoDecoder* decoder) {
        if (!decoder) {
            lua_createtable(L, 0, 0);
            return;
        }

        lua_createtable(L, 0, 10);
        
        lua_pushnumber(L, decoder->getDuration());
        lua_setfield(L, -2, "duration");
        
        lua_pushnumber(L, decoder->getCurrentTime());
        lua_setfield(L, -2, "time");
        
        lua_pushboolean(L, decoder->isLooping());
        lua_setfield(L, -2, "looping");

        double loop_end = 0.0, loop_duration = 0.0;
        decoder->getLoopRange(&loop_end, &loop_duration);
        lua_pushnumber(L, loop_end);
        lua_setfield(L, -2, "loop_end");
        lua_pushnumber(L, loop_duration);
        lua_setfield(L, -2, "loop_duration");
        
        auto size = decoder->getVideoSize();
        lua_pushinteger(L, size.x);
        lua_setfield(L, -2, "width");
        lua_pushinteger(L, size.y);
        lua_setfield(L, -2, "height");
        
        lua_pushinteger(L, static_cast<lua_Integer>(decoder->getVideoStreamIndex()));
        lua_setfield(L, -2, "video_stream");
        
        double frame_interval = decoder->getFrameInterval();
        lua_pushnumber(L, frame_interval);
        lua_setfield(L, -2, "frame_interval");
        
        if (frame_interval > 0.0) {
            lua_pushnumber(L, 1.0 / frame_interval);
            lua_setfield(L, -2, "fps");
        }
    }
}
