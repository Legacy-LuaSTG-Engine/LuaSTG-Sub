#include "luastg_config_generated.h"
#include "SteamAPI/SteamAPI.hpp"

#ifdef LUASTG_STEAM_API_ENABLE
#include "steam_api.h"
#endif

namespace luastg::SteamAPI
{
	bool Init() {
	#ifdef LUASTG_STEAM_API_ENABLE
	#ifdef LUASTG_STEAM_API_FORCE_LAUNCH_BY_STEAM
		if (::SteamAPI_RestartAppIfNecessary(LUASTG_STEAM_API_APP_ID)) {
			return false;
		}
	#endif
		if (!::SteamAPI_Init()) {
			return false;
		}
	#endif
		return true;
	}
	void RunCallbacks() {
	#ifdef LUASTG_STEAM_API_ENABLE
		::SteamAPI_RunCallbacks();
	#endif
	}
	void Shutdown() {
	#ifdef LUASTG_STEAM_API_ENABLE
		::SteamAPI_Shutdown();
	#endif
	}
}
