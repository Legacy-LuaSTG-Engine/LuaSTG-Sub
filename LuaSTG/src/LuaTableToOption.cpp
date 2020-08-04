#include "LuaTableToOption.hpp"

namespace LuaSTGPlus {
	game_option read_game_option_from_index(lua_State* L, int index) {
		// ?
		game_option op;
		if (lua_type(L, index) == LUA_TTABLE) {
			op.error_status = true;
			op.error_message = "Invalid option format, require lua table type.";
			return op;
		}
		// t
		lua_pushstring(L, "main");									// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TSTRING) {				// t s
			op.main_script = luaL_checkstring(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "title");									// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TSTRING) {				// t s
			op.window_name = luaL_checkstring(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "width");									// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TNUMBER) {				// t n
			op.window_width = (uint32_t)luaL_checkinteger(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "height");								// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TNUMBER) {				// t n
			op.window_height = (uint32_t)luaL_checkinteger(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "fullscreen");							// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TBOOLEAN) {				// t b
			op.window_fullscreen = (bool)lua_toboolean(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "vsync");									// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TBOOLEAN) {				// t b
			op.window_vsync = (bool)lua_toboolean(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "fps");									// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TNUMBER) {				// t n
			op.game_fps = (uint32_t)luaL_checkinteger(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "sevolume");								// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TNUMBER) {				// t n
			op.game_sevolume = (double)luaL_checknumber(L, index + 1);
		}
		lua_pop(L, 1);												// t

		lua_pushstring(L, "bgmvolume");								// t k
		lua_gettable(L, index);										// t ?
		if (lua_type(L, index + 1) == LUA_TNUMBER) {				// t n
			op.game_bgmvolume = (double)luaL_checknumber(L, index + 1);
		}
		lua_pop(L, 1);												// t

		return op;
	}

	game_option read_game_option_from_string(lua_State* L, const std::string& src) {
		int ret = 0;
		game_option op;
		// load
		ret = luaL_loadbuffer(L, src.data(), src.size(), "config");	// ?
		if (ret != 0) {												// errmsg
			op.error_status = true;
			op.error_message = luaL_optstring(L, 1, "Failed to parse config.");
			lua_pop(L, 1);
			return op;
		}
		// call														// chunk
		lua_newtable(L);											// chunk t
		lua_setglobal(L, "config");									// chunk
		ret = lua_pcall(L, 0, 0, 0);								// ?
		if (ret != 0) {												// errmsg
			op.error_status = true;
			op.error_message = luaL_optstring(L, 1, "Failed to parse config.");
			lua_pop(L, 1);											//
		}
		else {
			lua_getglobal(L, "config");								// t
			op = read_game_option_from_index(L, 1);
			lua_pop(L, 1);											//
		}
		lua_pushnil(L);												// nil
		lua_setglobal(L, "config");									//
		return op;
	}
}
