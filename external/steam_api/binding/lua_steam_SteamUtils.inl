#pragma once

struct xSteamUtils {
	static int GetAppID(lua_State* L) {
		lua_pushnumber(L, SteamUtils()->GetAppID());
		return 1;
	}
	static int GetCurrentBatteryPower(lua_State* L)
	{
		lua_pushnumber(L, SteamUtils()->GetCurrentBatteryPower());
		return 1;
	}
	static int GetIPCCallCount(lua_State* L)
	{
		lua_pushnumber(L, SteamUtils()->GetIPCCallCount());
		return 1;
	}
	static int GetIPCountry(lua_State* L)
	{
		lua_pushstring(L, SteamUtils()->GetIPCountry());
		return 1;
	}
	static int GetSecondsSinceAppActive(lua_State* L)
	{
		lua_pushnumber(L, SteamUtils()->GetSecondsSinceAppActive());
		return 1;
	}
	static int GetServerRealTime(lua_State* L)
	{
		lua_pushnumber(L, SteamUtils()->GetServerRealTime());
		return 1;
	}
	static int IsOverlayEnabled(lua_State* L)
	{
		lua_pushboolean(L, SteamUtils()->IsOverlayEnabled());
		return 1;
	}
	static int IsSteamChinaLauncher(lua_State* L)
	{
		lua_pushboolean(L, SteamUtils()->IsSteamChinaLauncher());
		return 1;
	}
	static int IsSteamInBigPictureMode(lua_State* L)
	{
		lua_pushboolean(L, SteamUtils()->IsSteamInBigPictureMode());
		return 1;
	}
	static int IsVRHeadsetStreamingEnabled(lua_State* L)
	{
		lua_pushboolean(L, SteamUtils()->IsVRHeadsetStreamingEnabled());
		return 1;
	}
	static int GetSteamUILanguage(lua_State* L)
	{
		lua_pushstring(L, SteamUtils()->GetSteamUILanguage());
		return 1;
	}
	static int xRegister(lua_State* L)
	{
		static const luaL_Reg lib[] = {
			xfbinding(GetAppID),
			xfbinding(GetCurrentBatteryPower),
			xfbinding(GetIPCCallCount),
			xfbinding(GetIPCountry),
			xfbinding(GetSecondsSinceAppActive),
			xfbinding(GetServerRealTime),
			xfbinding(IsOverlayEnabled),
			xfbinding(IsSteamChinaLauncher),
			xfbinding(IsSteamInBigPictureMode),
			xfbinding(IsVRHeadsetStreamingEnabled),
			xfbinding(GetSteamUILanguage),
			{NULL, NULL},
		};
		lua_pushstring(L, "SteamUtils");
		lua_createtable(L, 0, 11);
		luaL_register(L, NULL, lib);
		lua_settable(L, -3);
		return 0;
	};
};
