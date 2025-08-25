#pragma once



struct xSteamUserStats
{
	template<class T>
	class LuaCallResult {
	private:
		lua_State* m_L;
		SteamAPICall_t m_Callback;
		CCallResult<LuaCallResult<T>, T> m_CallResult;
		int m_luaCallbackRef;
	public:
		LuaCallResult(lua_State* L, SteamAPICall_t call, int luaCallbackRef, void (LuaCallResult<T>::* func)(T*, bool))
			: m_L(L), m_Callback(call), m_luaCallbackRef(luaCallbackRef)
		{
			m_CallResult.Set(m_Callback, this, func);
		}
		LuaCallResult() {
			m_CallResult.Set(k_uAPICallInvalid, this, &LuaCallResult<T>::NullCallBack);
		}
		~LuaCallResult() {
			if (m_luaCallbackRef != LUA_NOREF && m_L) {
				luaL_unref(m_L, LUA_REGISTRYINDEX, m_luaCallbackRef);
				m_luaCallbackRef = LUA_NOREF;
			}
			m_L = nullptr;
			m_Callback = k_uAPICallInvalid;
		}
		void RegisterCallback(lua_State* L, SteamAPICall_t call, int luaCallbackRef, void (LuaCallResult<T>::* func)(T*, bool)) {
			m_L = L;
			m_Callback = call;
			m_luaCallbackRef = luaCallbackRef;
			m_CallResult.Set(m_Callback, this, func);
		}
		void NullCallBack(T* pCallback, bool bIOFailure) {
			//delete this;
		}


#define CALLBACKFUNC(NAME, TOLUA) \
    void On##NAME(T* pCallback, bool bIOFailure) { \
        if (bIOFailure) { \
            luaL_error(m_L, "IO failure"); \
            return; \
        } \
        lua_pushstring(m_L, "SteamUserStats_callback_registry"); \
        lua_gettable(m_L, LUA_REGISTRYINDEX); \
        lua_rawgeti(m_L, -1, m_luaCallbackRef); \
        lua_remove(m_L, -2); \
        if (lua_isfunction(m_L, -1)) { \
            do { TOLUA; } while (0); \
            if (lua_pcall(m_L, 1, 0, 0) != LUA_OK) { \
                const char* errmsg = lua_tostring(m_L, -1); \
                luaL_error(m_L, "Lua callback error: %s", errmsg); \
				return; \
            } \
        } \
        else { \
            luaL_error(m_L, "Callback function not found"); \
        } \
        delete this; \
    }

		CALLBACKFUNC(LeaderboardScoreUploaded, {
			lua_createtable(m_L, 0, 5);
			lua_pushboolean(m_L, pCallback->m_bSuccess);
			lua_setfield(m_L, -2, "m_bSuccess");
			lua_push_SteamLeaderboard_t(m_L, pCallback->m_hSteamLeaderboard);
			lua_setfield(m_L, -2, "m_hSteamLeaderboard");
			lua_pushnumber(m_L, pCallback->m_nScore);
			lua_setfield(m_L, -2, "m_nScore");
			lua_pushboolean(m_L, pCallback->m_bScoreChanged);
			lua_setfield(m_L, -2, "m_bScoreChanged");
			lua_pushnumber(m_L, pCallback->m_nGlobalRankNew);
			lua_setfield(m_L, -2, "m_nGlobalRankNew");
			lua_pushnumber(m_L, pCallback->m_nGlobalRankPrevious);
			lua_setfield(m_L, -2, "m_nGlobalRankPrevious");
			});
		CALLBACKFUNC(LeaderboardFindResult, {
			lua_createtable(m_L, 0, 2);
			lua_push_SteamLeaderboard_t(m_L, pCallback->m_hSteamLeaderboard);
			lua_setfield(m_L, -2, "m_hSteamLeaderboard");
			lua_pushboolean(m_L, pCallback->m_bLeaderboardFound);
			lua_setfield(m_L, -2, "m_bLeaderboardFound");
			});
		CALLBACKFUNC(LeaderboardScoresDownloaded, {
			lua_createtable(m_L, 0, 3);
			lua_push_SteamLeaderboard_t(m_L, pCallback->m_hSteamLeaderboard);
			lua_setfield(m_L, -2, "m_hSteamLeaderboard");
			lua_push_SteamLeaderboardEntries_t(m_L, pCallback->m_hSteamLeaderboardEntries);
			lua_setfield(m_L, -2, "m_hSteamLeaderboardEntries");
			lua_pushnumber(m_L, pCallback->m_cEntryCount);
			lua_setfield(m_L, -2, "m_cEntryCount");
			});

	};

	static int getLuaFuncRef(lua_State* L, int idx) {
		if (idx == -1) {
			idx = lua_gettop(L);
		}
		luaL_checktype(L, idx, LUA_TFUNCTION);

		lua_pushstring(L, "SteamUserStats_callback_registry");
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushvalue(L, idx);
		int callbackRef = luaL_ref(L, -2);
		lua_pop(L, 1);
		return callbackRef;
	}
#define REGISTERCALLBACK(TYPE) \
    do { \
        if (result != k_uAPICallInvalid) { \
            new LuaCallResult<TYPE##_t>(L, result, callbackRef, &LuaCallResult<TYPE##_t>::On##TYPE); \
        } \
    } while(0)

	static int ResetAllStats(lua_State* L)  // callback
	{
		const bool bAchievementsToo = lua_toboolean(L, 1);
		const bool ret = SteamUserStats()->ResetAllStats(bAchievementsToo);
		lua_pushboolean(L, ret);
		return 1;
	};
	static int StoreStats(lua_State* L)  // callback
	{
		const bool ret = SteamUserStats()->StoreStats();
		lua_pushboolean(L, ret);
		return 1;
	};

	static int ClearAchievement(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const bool ret = SteamUserStats()->ClearAchievement(pchName);
		lua_pushboolean(L, ret);
		return 1;
	};
	static int GetAchievement(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		bool bAchieved = false;
		const bool ret = SteamUserStats()->GetAchievement(pchName, &bAchieved);
		lua_pushboolean(L, ret);
		lua_pushboolean(L, bAchieved);
		return 2;
	};
	static int GetAchievementAchievedPercent(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		float flPercent = 0.0f;
		const bool ret = SteamUserStats()->GetAchievementAchievedPercent(pchName, &flPercent);
		lua_pushboolean(L, ret);
		lua_pushnumber(L, flPercent);
		return 2;
	};
	static int GetAchievementAndUnlockTime(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		bool bAchieved = false;
		uint32 unUnlockTime = 0;
		const bool ret = SteamUserStats()->GetAchievementAndUnlockTime(pchName, &bAchieved, &unUnlockTime);
		lua_pushboolean(L, ret);
		lua_pushboolean(L, bAchieved);
		lua_push_uint32(L, unUnlockTime);
		return 3;
	};
	static int GetAchievementDisplayAttribute(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const char* pchKey = luaL_checkstring(L, 2);
		const char* ret = SteamUserStats()->GetAchievementDisplayAttribute(pchName, pchKey);
		lua_pushstring(L, ret);
		return 1;
	};
	static int GetAchievementIcon(lua_State* L) // !
	{
		return 0;
	};
	static int GetAchievementName(lua_State* L)
	{
		const uint32 iAchievement = lua_to_uint32(L, 1);
		const char* ret = SteamUserStats()->GetAchievementName(iAchievement);
		lua_pushstring(L, ret);
		return 1;
	};
	static int GetMostAchievedAchievementInfo(lua_State* L) // !
	{
		return 0;
	};
	static int GetNextMostAchievedAchievementInfo(lua_State* L) // !
	{
		return 0;
	};
	static int GetNumAchievements(lua_State* L)
	{
		const uint32 ret = SteamUserStats()->GetNumAchievements();
		lua_push_uint32(L, ret);
		return 1;
	};
	static int GetUserAchievement(lua_State* L) // !
	{
		return 0;
	};
	static int GetUserAchievementAndUnlockTime(lua_State* L) // !
	{
		return 0;
	};
	static int IndicateAchievementProgress(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const uint32 nCurProgress = lua_to_uint32(L, 2);
		const uint32 nMaxProgress = lua_to_uint32(L, 3);
		const bool ret = SteamUserStats()->IndicateAchievementProgress(pchName, nCurProgress, nMaxProgress);
		lua_pushboolean(L, ret);
		return 1;
	};
	static int RequestGlobalAchievementPercentages(lua_State* L) // !
	{
		return 0;
	};
	static int SetAchievement(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const bool ret = SteamUserStats()->SetAchievement(pchName);
		lua_pushboolean(L, ret);
		return 1;
	};

	static int GetGlobalStat(lua_State* L) // !
	{
		return 0;
	};
	static int GetGlobalStatHistory(lua_State* L) // !
	{
		return 0;
	};
	static int GetStat(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const char* type = luaL_checkstring(L, 2);
		if (str_equal(type, "int32", 6))
		{
			int32 Data = 0;
			const bool ret = SteamUserStats()->GetStat(pchName, &Data);
			lua_pushboolean(L, ret);
			lua_pushinteger(L, Data);
			return 2;
		}
		else if (str_equal(type, "float", 6))
		{
			float Data = 0.0f;
			const bool ret = SteamUserStats()->GetStat(pchName, &Data);
			lua_pushboolean(L, ret);
			lua_pushnumber(L, Data);
			return 2;
		}
		else
		{
			return luaL_error(L, "bad argument #2 (\"int32\" or \"float\" expected, got \"%s\")", type);
		}
	};
	static int GetUserStat(lua_State* L) // !
	{
		return 0;
	};
	static int RequestGlobalStats(lua_State* L) // !
	{
		return 0;
	};
	static int RequestUserStats(lua_State* L) // !
	{
		return 0;
	};
	static int SetStat(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const char* type = luaL_checkstring(L, 2);
		if (str_equal(type, "int32", 6))
		{
			const int32 Data = luaL_checkinteger(L, 3);
			const bool ret = SteamUserStats()->SetStat(pchName, Data);
			lua_pushboolean(L, ret);
			return 1;
		}
		else if (str_equal(type, "float", 6))
		{
			const float Data = (float)luaL_checknumber(L, 3);
			const bool ret = SteamUserStats()->SetStat(pchName, Data);
			lua_pushboolean(L, ret);
			return 1;
		}
		else
		{
			return luaL_error(L, "bad argument #2 (\"int32\" or \"float\" expected, got \"%s\")", type);
		}
	};
	static int UpdateAvgRateStat(lua_State* L)
	{
		const char* pchName = luaL_checkstring(L, 1);
		const float flCountThisSession = (float)luaL_checknumber(L, 2);
		const double dSessionLength = luaL_checknumber(L, 3);
		const bool ret = SteamUserStats()->UpdateAvgRateStat(pchName, flCountThisSession, dSessionLength);
		lua_pushboolean(L, ret);
		return 1;
	};

	static int AttachLeaderboardUGC(lua_State* L) // !
	{
		return 0;
	};
	static int DownloadLeaderboardEntries(lua_State* L) // !
	{
		SteamLeaderboard_t hSteamLeaderboard = (SteamLeaderboard_t)lua_to_SteamLeaderboard_t(L, 1);
		ELeaderboardDataRequest eLeaderboardDataRequest = (ELeaderboardDataRequest)lua_tointeger(L, 2);
		int nRangeStart = lua_tonumber(L, 3);
		int nRangeEnd = lua_tonumber(L, 4);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUserStats()->DownloadLeaderboardEntries(hSteamLeaderboard, eLeaderboardDataRequest, nRangeStart, nRangeEnd);
		REGISTERCALLBACK(LeaderboardScoresDownloaded);
		return 0;
	};
	static int DownloadLeaderboardEntriesForUsers(lua_State* L) // !
	{
		return 0;
	};
	static int FindLeaderboard(lua_State* L) // !
	{
		const char* pchLeaderboardName = luaL_checkstring(L, 1);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUserStats()->FindLeaderboard(pchLeaderboardName);
		REGISTERCALLBACK(LeaderboardFindResult);
		return 0;
	};
	static int FindOrCreateLeaderboard(lua_State* L) // !
	{
		const char* pchLeaderboardName = luaL_checkstring(L, 1);
		ELeaderboardSortMethod eLeaderboardSortMethod = (ELeaderboardSortMethod)lua_tointeger(L, 2);
		ELeaderboardDisplayType eLeaderboardDisplayType = (ELeaderboardDisplayType)lua_tointeger(L, 3);
		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUserStats()->FindOrCreateLeaderboard(pchLeaderboardName, eLeaderboardSortMethod, eLeaderboardDisplayType);
		REGISTERCALLBACK(LeaderboardFindResult);
		return 0;
	};
	static int GetDownloadedLeaderboardEntry(lua_State* L) // !
	{
		SteamLeaderboardEntries_t hSteamLeaderboardEntries = (SteamLeaderboardEntries_t)lua_to_SteamLeaderboardEntries_t(L, 1);
		int count = lua_tonumber(L, 2);
		int cDetailsMax=lua_tonumber(L, 3);
		lua_createtable(L, count, 0);
		for (int index = 0; index < count; index++)
		{
			LeaderboardEntry_t entry;
			int* pDetails = new int[cDetailsMax];
			if (SteamUserStats()->GetDownloadedLeaderboardEntry(hSteamLeaderboardEntries, index, &entry, pDetails, cDetailsMax))
			{
				lua_createtable(L, 0, 6);
				lua_push_uint32(L, entry.m_steamIDUser.GetAccountID());
				lua_setfield(L, -2, "m_steamIDUser");
				lua_pushinteger(L, entry.m_nGlobalRank);
				lua_setfield(L, -2, "m_nGlobalRank");
				lua_pushnumber(L, entry.m_nScore);
				lua_setfield(L, -2, "m_nScore");
				lua_pushnumber(L, entry.m_cDetails);
				lua_setfield(L, -2, "m_cDetails");
				lua_push_UGCHandle_t(L, entry.m_hUGC);
				lua_setfield(L, -2, "m_hUGC");
				lua_createtable(L, entry.m_cDetails, 0);
				for (int i = 0; i < entry.m_cDetails; i++)
				{
					lua_pushinteger(L, pDetails[i]);
					lua_rawseti(L, -2, i + 1);
				}
				lua_setfield(L, -2, "m_pDetails");
			}
			else {
				lua_pushnil(L);
			}
			lua_rawseti(L, -2, index + 1);
			delete[] pDetails;
		}
		return 1;
	};
	static int GetLeaderboardDisplayType(lua_State* L) // !
	{
		return 0;
	};
	static int GetLeaderboardEntryCount(lua_State* L) // !
	{
		return 0;
	};
	static int GetLeaderboardName(lua_State* L) // !
	{
		return 0;
	};
	static int GetLeaderboardSortMethod(lua_State* L) // !
	{
		return 0;
	};
	static int UploadLeaderboardScore(lua_State* L) // !
	{
		SteamLeaderboard_t hSteamLeaderboard = (SteamLeaderboard_t)lua_to_SteamLeaderboard_t(L, 1);
		ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod = (ELeaderboardUploadScoreMethod)lua_tointeger(L, 2);
		int detailCount = 0;
		int* pDetails = nullptr;
		if (lua_istable(L, 4)) {
			detailCount = (int)lua_objlen(L, 4);
			pDetails = new int[detailCount];
			for (int i = 0; i < detailCount; ++i) {
				lua_rawgeti(L, 4, i + 1);  // Lua 索引从1开始
				if (lua_isnumber(L, -1)) {
					pDetails[i] = (int)lua_tointeger(L, -1);
				}
				else {
					pDetails[i] = 0; // 默认值，或者你也可以抛错
				}
				lua_pop(L, 1);
			}
		}

		int callbackRef = getLuaFuncRef(L, -1);
		auto const result = SteamUserStats()->UploadLeaderboardScore(hSteamLeaderboard, 
			eLeaderboardUploadScoreMethod, lua_tonumber(L, 3), pDetails, detailCount);
		delete[] pDetails;
		REGISTERCALLBACK(LeaderboardScoreUploaded);
		return 0;
	};

	static int GetNumberOfCurrentPlayers(lua_State* L) // callback
	{
		const SteamAPICall_t ret = SteamUserStats()->GetNumberOfCurrentPlayers();
		lua_push_SteamAPICall_t(L, ret);
		return 1;
	};

	static int xRegister(lua_State* L)
	{
		static const luaL_Reg lib[] = {
			xfbinding(ResetAllStats),
			xfbinding(StoreStats),

			xfbinding(ClearAchievement),
			xfbinding(GetAchievement),
			xfbinding(GetAchievementAchievedPercent),
			xfbinding(GetAchievementAndUnlockTime),
			xfbinding(GetAchievementDisplayAttribute),
			xfbinding(GetAchievementIcon),
			xfbinding(GetAchievementName),
			xfbinding(GetMostAchievedAchievementInfo),
			xfbinding(GetNextMostAchievedAchievementInfo),
			xfbinding(GetNumAchievements),
			xfbinding(GetUserAchievement),
			xfbinding(GetUserAchievementAndUnlockTime),
			xfbinding(IndicateAchievementProgress),
			xfbinding(RequestGlobalAchievementPercentages),
			xfbinding(SetAchievement),

			xfbinding(GetGlobalStat),
			xfbinding(GetGlobalStatHistory),
			xfbinding(GetStat),
			xfbinding(GetUserStat),
			xfbinding(RequestGlobalStats),
			xfbinding(RequestUserStats),
			xfbinding(SetStat),
			xfbinding(UpdateAvgRateStat),

			xfbinding(AttachLeaderboardUGC),
			xfbinding(DownloadLeaderboardEntries),
			xfbinding(DownloadLeaderboardEntriesForUsers),
			xfbinding(FindLeaderboard),
			xfbinding(FindOrCreateLeaderboard),
			xfbinding(GetDownloadedLeaderboardEntry),
			xfbinding(GetLeaderboardDisplayType),
			xfbinding(GetLeaderboardEntryCount),
			xfbinding(GetLeaderboardName),
			xfbinding(GetLeaderboardSortMethod),
			xfbinding(UploadLeaderboardScore),

			xfbinding(GetNumberOfCurrentPlayers),

			{NULL, NULL},
		};
		lua_pushstring(L, "SteamUserStats");
		lua_createtable(L, 0, 38);
		luaL_register(L, NULL, lib);
		lua_settable(L, -3);


		lua_newtable(L);
		lua_pushstring(L, "SteamUserStats_callback_registry");
		lua_pushvalue(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
		lua_pop(L, 1);
		return 0;
	};
#undef CALLBACKFUNC
#undef REGISTERCALLBACK
};
