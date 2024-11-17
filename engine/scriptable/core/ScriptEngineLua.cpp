#include "core/ScriptEngine.hpp"
#include <cassert>
#include <format>
#include <array>
#include <memory_resource>
#include "lua.hpp"
#ifdef LUASTG_LINK_LUAJIT
#include "luajit.h"
#endif

using namespace std::string_view_literals;

namespace core {
#define LVM static_cast<lua_State*>(handle)

	static void* debug_trackback_key{};
	static void* stack_trackback_function_name{};

	static int stackTraceBack(lua_State* L) {
		std::array<char, 1024> buf{};
		std::pmr::monotonic_buffer_resource res(buf.data(), buf.size());
		std::pmr::string msg(&res);

		size_t l{};
		if (char const* m = lua_tolstring(L, 1, &l); m) {
			msg.append(m, l);
		}
		else if (luaL_callmeta(L, 1, "__tostring")) {
			m = lua_tolstring(L, -1, &l);
			msg.append("(error object is a '"sv);
			msg.append(m, l);
			msg.append("' value)"sv);
			lua_pop(L, 1);
		}
		else {
			msg.append("(error object is a "sv);
			if (lua_isnoneornil(L, 1)) {
				msg.append("(error object is a nil value)"sv);
			}
			else if (lua_isboolean(L, 1)) {
				msg.append(lua_toboolean(L, 1) ? "true"sv : "false"sv);
			}
			else if (lua_istable(L, 1)) {
				msg.append("table"sv);
			}
			else if (lua_islightuserdata(L, 1)) {
				msg.append("light userdata"sv);
			}
			else if (lua_isuserdata(L, 1)) {
				msg.append("userdata"sv);
			}
			else if (lua_iscfunction(L, 1)) {
				msg.append("C function"sv);
			}
			else if (lua_isfunction(L, 1)) {
				msg.append("function"sv);
			}
			else if (lua_isthread(L, 1)) {
				msg.append("thread"sv);
			}
			else {
				msg.append("?"sv);
			}
			msg.append(" value)"sv);
		}

		msg.append("\nstack traceback:"sv);

		lua_Debug debug{};
		for (int level = 1; lua_getstack(L, level, &debug); level += 1) {
			lua_getinfo(L, "Sfln", &debug);
			msg.append("\n\t"sv);
			msg.append(debug.short_src);
			msg.append(":"sv);
			if (debug.currentline > 0) {
				char line_number[22]{};
				int n = std::snprintf(line_number, 22, "%d", debug.currentline);
				msg.append(line_number, static_cast<size_t>(n));
				msg.append(":"sv);
			}
			if (debug.name && debug.name[0]) {
				msg.append(" in"sv);
				if (debug.namewhat && debug.namewhat[0]) {
					msg.append(" ("sv);
					msg.append(debug.namewhat);
					msg.append(")"sv);
				}
				msg.append(" function '"sv);
				msg.append(debug.name);
				msg.append("'"sv);
			}
			else if (debug.what) {
				if (debug.what[0] == 'm' /* "main" */) {
					msg.append(" in main chunk"sv);
				}
				else if (debug.what[0] == 'C' /* "C" */) {
					msg.append(" at C"sv);
				}
				else {
					lua_pushlightuserdata(L, &stack_trackback_function_name);
					lua_gettable(L, LUA_REGISTRYINDEX);
					if (debug.what[0] == 'L' /* "Lua" */ && lua_isstring(L, -1)) {
						size_t len{};
						auto const* str = lua_tolstring(L, -1, &len);
						msg.append(" in (global) function '"sv);
						msg.append(str, len);
						msg.append("'"sv);
					}
					else {
						msg.append(" in function <"sv);
						msg.append(debug.short_src);
						msg.append(":"sv);
						char line_number[22]{};
						int n = std::snprintf(line_number, 22, "%d", debug.linedefined);
						msg.append(line_number, static_cast<size_t>(n));
						msg.append(">"sv);
					}
					lua_pop(L, 1);
				}
			}
			else {
				msg.append(" unknown"sv);
			}
		}

		lua_pushlstring(L, msg.c_str(), msg.length());

		return 1;
	}

	ScriptEngine::CallResult ScriptEngine::loadFromFile(std::string_view const& path) {
		assert(handle);
		constexpr auto function_name = "dofile"sv;

		// -------------------- get stack traceback function

		lua_pushlightuserdata(LVM, &stackTraceBack);
		// ^stack: ... *key
		lua_gettable(LVM, LUA_REGISTRYINDEX);
		// ^stack: ... trackback
		auto const debug_trackback_index = lua_gettop(LVM);

		// -------------------- get calling function from global

		lua_pushlstring(LVM, function_name.data(), function_name.size());
		// ^stack: ... trackback "FUNCTION"
		lua_gettable(LVM, LUA_GLOBALSINDEX);
		// ^stack: ... trackback FUNCTION?

		// -------------------- call

		lua_pushlstring(LVM, path.data(), path.size());
		// ^stack: ... trackback FUNCTION? "path"
		auto const r = lua_pcall(LVM, 1, 0, debug_trackback_index);
		// ^stack: ... trackback error?

		// -------------------- check call result

		if (r != 0) {
			if (r == LUA_ERRMEM) {
				return CallResult("memory allocation error"sv);
			}
			else if (r == LUA_ERRERR) {
				return CallResult("error while running the error handler function"sv);
			}
			else {
				size_t l{};
				if (char const* m = lua_tolstring(LVM, -1, &l); m) {
					return CallResult(l > 0 ? std::string_view{ m, l } : "unknown error"sv);
				}
				else {
					return CallResult("(error object is a nil value)"sv);
				}
			}
		}

		// -------------------- clean

		lua_settop(LVM, debug_trackback_index - 1);
		// ^stack: ...

		return CallResult(true);
	}
	ScriptEngine::CallResult ScriptEngine::call(std::string_view const& function_name) {
		assert(handle);

		// -------------------- get stack traceback function

		lua_pushlightuserdata(LVM, &stackTraceBack);
		// ^stack: ... *key
		lua_gettable(LVM, LUA_REGISTRYINDEX);
		// ^stack: ... trackback
		auto const debug_trackback_index = lua_gettop(LVM);

		// -------------------- set calling function name

		lua_pushlightuserdata(LVM, &stack_trackback_function_name);
		// ^stack: ... trackback *key
		lua_pushlstring(LVM, function_name.data(), function_name.size());
		// ^stack: ... trackback *key "FUNCTION"
		lua_settable(LVM, LUA_REGISTRYINDEX);
		// ^stack: ... trackback

		// -------------------- get calling function from global

		lua_pushlstring(LVM, function_name.data(), function_name.size());
		// ^stack: ... trackback "FUNCTION"
		lua_gettable(LVM, LUA_GLOBALSINDEX);
		// ^stack: ... trackback FUNCTION?

		// -------------------- call

		auto const r = lua_pcall(LVM, 0, 0, debug_trackback_index);
		// ^stack: ... trackback error?

		// -------------------- clean calling function name

		lua_pushlightuserdata(LVM, &stack_trackback_function_name);
		// ^stack: ... trackback error? *key
		lua_pushnil(LVM);
		// ^stack: ... trackback error? *key nil
		lua_settable(LVM, LUA_REGISTRYINDEX);
		// ^stack: ... trackback error?

		// -------------------- check call result

		if (r != 0) {
			if (r == LUA_ERRMEM) {
				return CallResult("memory allocation error"sv);
			}
			else if (r == LUA_ERRERR) {
				return CallResult("error while running the error handler function"sv);
			}
			else {
				size_t l{};
				if (char const* m = lua_tolstring(LVM, -1, &l); m) {
					return CallResult(l > 0 ? std::string_view{ m, l } : "unknown error"sv);
				}
				else {
					return CallResult("(error object is a nil value)"sv);
				}
			}
		}

		// -------------------- clean

		lua_settop(LVM, debug_trackback_index - 1);
		// ^stack: ...

		return CallResult(true);
	}

	bool ScriptEngine::open() {
		handle = static_cast<void*>(luaL_newstate());
		if (!handle) {
			return false;
		}
	#ifdef LUASTG_LINK_LUAJIT
		if (0 == luaJIT_setmode(LVM, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON)) {
			// TODO; write log
		}
	#endif // LUASTG_LINK_LUAJIT
		luaL_openlibs(LVM);
		return registerStackTraceBackHandler();
	}
	void ScriptEngine::close() {
		if (LVM) {
			lua_close(LVM);
			handle = nullptr;
		}
	}

	bool ScriptEngine::registerStackTraceBackHandler() {
		assert(handle);

		// -------------------- store debug.traceback

		// ^stack: ...
		lua_getglobal(LVM, "require");
		// ^stack: ... require
		if (!lua_isfunction(LVM, lua_gettop(LVM))) {
			return false;
		}
		lua_pushstring(LVM, "debug");
		// ^stack: ... require "debug"
		lua_call(LVM, 1, 1);
		// ^stack: ... debug?
		int const debug_index = lua_gettop(LVM);
		if (!lua_istable(LVM, debug_index)) {
			return false;
		}
		// ^stack: ... debug
		lua_pushlightuserdata(LVM, &debug_trackback_key);
		// ^stack: ... debug *key
		lua_getfield(LVM, debug_index, "traceback");
		// ^stack: ... debug *key traceback
		if (!lua_isfunction(LVM, debug_index + 2)) {
			return false;
		}
		lua_settable(LVM, LUA_REGISTRYINDEX);
		// ^stack: ... debug
		lua_pop(LVM, 1);
		// ^stack: ...

		// -------------------- register global stack traceback function

		lua_pushlightuserdata(LVM, &stackTraceBack);
		// ^stack: ... *key
		lua_pushcfunction(LVM, &stackTraceBack);
		// ^stack: ... *key stackTraceBack
		lua_settable(LVM, LUA_REGISTRYINDEX);
		// ^stack: ...

		return true;
	}
}
