#include "LuaBinding/LuaWrapper.hpp"
#include "Platform/KnownDirectory.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>
#include "utf8.hpp"

void LuaSTGPlus::LuaWrapper::PlatformWrapper::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		static int GetLocalAppDataPath(lua_State* L) noexcept
		{
			try
			{
				std::string path;
				if (Platform::KnownDirectory::getLocalAppData(path))
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
				if (Platform::KnownDirectory::getRoamingAppData(path))
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
		static int api_MessageBox(lua_State* L)
		{
			char const* title = luaL_checkstring(L, 1);
			char const* text = luaL_checkstring(L, 2);
			UINT flags = (UINT)luaL_checkinteger(L, 3);
			int result = MessageBoxW(
				(LAPP.GetAppModel() && LAPP.GetAppModel()->getWindow()) ? (HWND)LAPP.GetAppModel()->getWindow()->getNativeHandle() : NULL,
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
		{ "Execute", &Wrapper::Execute },
		{ "MessageBox", &Wrapper::api_MessageBox },
		{ NULL, NULL },
	};

	luaL_Reg const lib_empty[] = {
		{ NULL, NULL },
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib);             // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".Platform", lib); // ??? lstg lstg.Platform
	lua_setfield(L, -1, "Platform");                       // ??? lstg
	lua_pop(L, 1);                                         // ???
}
