#include "LuaBinding/LuaWrapper.hpp"
#include "lua/plus.hpp"
#include "ApplicationRestart.hpp"
#include "Platform/KnownDirectory.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>
#include "utf8.hpp"

namespace {
	HWND getMainWindow(core::Graphics::IWindow* const window) {
		if (window == nullptr) {
			return nullptr;
		}
		return static_cast<HWND>(window->getNativeHandle());
	}
}

void luastg::binding::Platform::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		static int GetLocalAppDataPath(lua_State* L) noexcept
		{
			try
			{
				std::string path;
				if (::Platform::KnownDirectory::getLocalAppData(path))
				{
					lua_pushstring(L, path.c_str());
				}
				else
				{
					lua_pushstring(L, "");
				}
			}
			catch (const std::bad_alloc&)
			{
				lua_pushstring(L, "");
			}
			return 1;
		}
		static int GetRoamingAppDataPath(lua_State* L) noexcept
		{
			try
			{
				std::string path;
				if (::Platform::KnownDirectory::getRoamingAppData(path))
				{
					lua_pushstring(L, path.c_str());
				}
				else
				{
					lua_pushstring(L, "");
				}
			}
			catch (const std::bad_alloc&)
			{
				lua_pushstring(L, "");
			}
			return 1;
		}
		static int RestartWithCommandLineArguments(lua_State* const vm) noexcept {
			lua::stack_t const ctx(vm);
			std::vector<std::string> args;
			if (ctx.is_table(1)) {
				if (auto const n = ctx.get_array_size(1); n > 0) {
					args.resize(n);
					for (size_t i = 0; i < ctx.get_array_size(1); i += 1) {
						args[i].assign(ctx.get_array_value<std::string_view>(1, static_cast<int32_t>(i + 1)));
					}
				}
			}
			ApplicationRestart::enableWithCommandLineArguments(args);
			return 0;
		}
#ifdef LUASTG_ENABLE_EXECUTE_API
		static int Execute(lua_State* L) noexcept
		{
			struct Detail_
			{
				static bool Execute(const char* path, const char* args, const char* directory, bool bWait, bool bShow) noexcept
				{
					std::wstring tPath, tArgs, tDirectory;

					try
					{
						tPath = utf8::to_wstring(path);
						tArgs = utf8::to_wstring(args);
						if (directory)
							tDirectory = utf8::to_wstring(directory);

						SHELLEXECUTEINFO tShellExecuteInfo;
						memset(&tShellExecuteInfo, 0, sizeof(SHELLEXECUTEINFO));

						tShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
						tShellExecuteInfo.fMask = bWait ? SEE_MASK_NOCLOSEPROCESS : 0;
						tShellExecuteInfo.lpVerb = L"open";
						tShellExecuteInfo.lpFile = tPath.c_str();
						tShellExecuteInfo.lpParameters = tArgs.c_str();
						tShellExecuteInfo.lpDirectory = directory ? tDirectory.c_str() : nullptr;
						tShellExecuteInfo.nShow = bShow ? SW_SHOWDEFAULT : SW_HIDE;
						
						if (FALSE == ShellExecuteEx(&tShellExecuteInfo))
							return false;

						if (bWait)
						{
							WaitForSingleObject(tShellExecuteInfo.hProcess, INFINITE);
							CloseHandle(tShellExecuteInfo.hProcess);
						}
						return true;
					}
					catch (const std::bad_alloc&)
					{
						return false;
					}
				}
			};

			const char* path = luaL_checkstring(L, 1);
			const char* args = luaL_optstring(L, 2, "");
			const char* directory = luaL_optstring(L, 3, NULL);
			bool bWait = true;
			bool bShow = true;
			if (lua_gettop(L) >= 4)
				bWait = lua_toboolean(L, 4) == 0 ? false : true;
			if (lua_gettop(L) >= 5)
				bShow = lua_toboolean(L, 5) == 0 ? false : true;
			
			lua_pushboolean(L, Detail_::Execute(path, args, directory, bWait, bShow));
			return 1;
		}
#endif
		static int api_MessageBox(lua_State* L)
		{
			char const* title = luaL_checkstring(L, 1);
			char const* text = luaL_checkstring(L, 2);
			UINT flags = (UINT)luaL_checkinteger(L, 3);
			int result = MessageBoxW(
				getMainWindow(LAPP.getWindow()),
				utf8::to_wstring(text).c_str(),
				utf8::to_wstring(title).c_str(),
				flags);
			lua_pushinteger(L, result);
			return 1;
		}
	};

	luaL_Reg const lib[] = {
		{ "GetLocalAppDataPath", &Wrapper::GetLocalAppDataPath },
		{ "GetRoamingAppDataPath", &Wrapper::GetRoamingAppDataPath },
		{ "RestartWithCommandLineArguments", &Wrapper::RestartWithCommandLineArguments },
	#ifdef LUASTG_ENABLE_EXECUTE_API
		{ "Execute", &Wrapper::Execute },
	#endif
		{ "MessageBox", &Wrapper::api_MessageBox },
		{ NULL, NULL },
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib);             // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".Platform", lib); // ??? lstg lstg.Platform
	lua_setfield(L, -1, "Platform");                       // ??? lstg
	lua_pop(L, 1);                                         // ???

#ifndef LUASTG_ENABLE_EXECUTE_API
	constexpr luaL_Reg empty[]{{}};

	luaL_register(L, LUA_OSLIBNAME, empty); // ??? os
	lua_pushstring(L, "execute");           // ??? os "execute"
	lua_pushnil(L);                         // ??? os "execute" nil
	lua_settable(L, -3);                    // ??? os
	lua_pop(L, 1);                          // ???

	luaL_register(L, LUA_IOLIBNAME, empty); // ??? io
	lua_pushstring(L, "popen");             // ??? io "popen"
	lua_pushnil(L);                         // ??? io "popen" nil
	lua_settable(L, -3);                    // ??? io
	lua_pop(L, 1);                          // ???
#endif
}
