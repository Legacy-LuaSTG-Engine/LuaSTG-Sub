#include "LuaWrapper/LuaWrapper.hpp"
#include "Common/SystemDirectory.hpp"
#define NOMINMAX
#include <Windows.h>

void LuaSTGPlus::LuaWrapper::PlatformWrapper::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		static int GetLocalAppDataPath(lua_State* L)LNOEXCEPT
		{
			try {
				std::wstring wpath;
				if (windows::getLocalAppDataDirectory(wpath))
				{
					std::string path = fcyStringHelper::WideCharToMultiByte(wpath, CP_UTF8);
					lua_pushstring(L, path.c_str());
				}
				else
				{
					lua_pushstring(L, "");
				}
			}
			catch (const std::bad_alloc&) {
				lua_pushstring(L, "");
			}
			return 1;
		}
		static int GetRoamingAppDataPath(lua_State* L)LNOEXCEPT
		{
			try {
				std::wstring wpath;
				if (windows::getRoamingAppDataDirectory(wpath))
				{
					std::string path = fcyStringHelper::WideCharToMultiByte(wpath, CP_UTF8);
					lua_pushstring(L, path.c_str());
				}
				else
				{
					lua_pushstring(L, "");
				}
			}
			catch (const std::bad_alloc&) {
				lua_pushstring(L, "");
			}
			return 1;
		}
		static int Execute(lua_State* L)LNOEXCEPT
		{
			struct Detail_
			{
				LNOINLINE static bool Execute(const char* path, const char* args, const char* directory, bool bWait, bool bShow)LNOEXCEPT
				{
					std::wstring tPath, tArgs, tDirectory;

					try
					{
						tPath = fcyStringHelper::MultiByteToWideChar(path);
						tArgs = fcyStringHelper::MultiByteToWideChar(args);
						if (directory)
							tDirectory = fcyStringHelper::MultiByteToWideChar(directory);

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
	};

	luaL_Reg const lib[] = {
		{ "GetLocalAppDataPath", &Wrapper::GetLocalAppDataPath },
		{ "GetRoamingAppDataPath", &Wrapper::GetRoamingAppDataPath },
		{ "Execute", &Wrapper::Execute },
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
