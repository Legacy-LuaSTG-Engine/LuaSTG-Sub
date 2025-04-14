#pragma once

struct xSteamFriends {
	static int  ActivateGameOverlay(lua_State* L) {
		const char* pchDialog = luaL_checkstring(L, 1);
		SteamFriends()->ActivateGameOverlay(pchDialog);
		return 1;
	}
	static int ActivateGameOverlayToWebPage(lua_State* L) {
		const char* pchURL = luaL_checkstring(L, 1);
		SteamFriends()->ActivateGameOverlayToWebPage(pchURL);
		return 1;
	}

	static int xRegister(lua_State* L)
	{
		static const luaL_Reg lib[] = {
			xfbinding(ActivateGameOverlay),
			xfbinding(ActivateGameOverlayToWebPage),
			{NULL, NULL},
		};
		lua_pushstring(L, "SteamFriends");
		lua_createtable(L, 0, 3);
		luaL_register(L, NULL, lib);
		lua_settable(L, -3);
		return 0;
	};
};