#include "SteamAPI.hpp"
#include "Config.h"

#ifdef USING_STEAM_API
#pragma comment(lib, "steam_api.lib")
#include "steam_api.h"
#endif // USING_STEAM_API

namespace LuaSTGPlus {
	bool SteamAPI_Init() {
		#ifdef USING_STEAM_API
			#ifdef KEEP_LAUNCH_BY_STEAM
				if (::SteamAPI_RestartAppIfNecessary(STEAM_APP_ID)) {
					return false;
				}
			#endif // KEEP_LAUNCH_BY_STEAM
			if (!::SteamAPI_Init()) {
				return false;
			}
			return true;
		#else  // USING_STEAM_API
			return true; // always return true
		#endif // USING_STEAM_API
	}

	void SteamAPI_RunCallbacks() {
		#ifdef USING_STEAM_API
			::SteamAPI_RunCallbacks();
		#endif // USING_STEAM_API
	}

	void SteamAPI_Shutdown() {
		#ifdef USING_STEAM_API
			::SteamAPI_Shutdown();
		#endif // USING_STEAM_API
	}
};
