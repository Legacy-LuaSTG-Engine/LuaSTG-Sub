#include "SteamAPI/SteamAPI.hpp"

#ifdef HAVE_STEAM_API
#include "steam_api.h"
#endif

namespace luastg::SteamAPI
{
	bool Init()
	{
	#ifdef HAVE_STEAM_API
	#ifdef USING_STEAM_API
	#ifdef KEEP_LAUNCH_BY_STEAM
		if (::SteamAPI_RestartAppIfNecessary(STEAM_APP_ID))
		{
			return false;
		}
	#endif
		if (!::SteamAPI_Init())
		{
			return false;
		}
	#endif
	#endif
		return true;
	}
	void RunCallbacks()
	{
	#ifdef HAVE_STEAM_API
	#ifdef USING_STEAM_API
		::SteamAPI_RunCallbacks();
	#endif
	#endif
	}
	void Shutdown()
	{
	#ifdef HAVE_STEAM_API
	#ifdef USING_STEAM_API
		::SteamAPI_Shutdown();
	#endif
	#endif
	}
}
