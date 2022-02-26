#pragma once
#include <string>
#include "lua.hpp"

namespace LuaSTGPlus {
	struct game_option {
		std::string main_script = "src/main.lua";	// main
		std::string window_name = "LuaSTG";			// title
		uint32_t window_width = 640u;				// width
		uint32_t window_height = 480u;				// height
		bool window_fullscreen = false;				// fullscreen
		bool window_vsync = false;					// vsync
		uint32_t game_fps = 60u;					// fps
		double game_sevolume = 100.0;				// sevolume
		double game_bgmvolume = 100.0;				// bgmvolume
		bool error_status = false;
		std::string error_message = "";
	};

	game_option read_game_option_from_index(lua_State* L, int index);

	game_option read_game_option_from_string(lua_State* L, const std::string& src);
}
