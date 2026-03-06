#pragma once
#include "core/VideoDecoder.hpp"
#include "core/Texture2D.hpp"
#include "lua/plus.hpp"
#include <vector>

namespace luastg::binding::video {
    // Parse VideoOpenOptions from Lua table
    void parseVideoOptions(lua_State* vm, int index, core::VideoOpenOptions& opt);

    // Push video stream info array to Lua stack
    void pushVideoStreamsToLua(lua_State* vm, core::IVideoDecoder* decoder);

    // Push audio stream info array to Lua stack
    void pushAudioStreamsToLua(lua_State* vm, core::IVideoDecoder* decoder);

    // Push video info table to Lua stack
    void pushVideoInfoToLua(lua_State* vm, core::IVideoDecoder* decoder);
}
