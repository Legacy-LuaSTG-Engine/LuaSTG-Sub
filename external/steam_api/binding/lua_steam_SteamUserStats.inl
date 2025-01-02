#pragma once

struct xSteamUserStats
{
    static int ResetAllStats      (lua_State* L)  // callback
    {
        const bool bAchievementsToo = lua_toboolean(L, 1);
        const bool ret = SteamUserStats()->ResetAllStats(bAchievementsToo);
        lua_pushboolean(L, ret);
        return 1;
    };
    static int StoreStats         (lua_State* L)  // callback
    {
        const bool ret = SteamUserStats()->StoreStats();
        lua_pushboolean(L, ret);
        return 1;
    };
    
    static int ClearAchievement                   (lua_State* L)
    {
        const char *pchName = luaL_checkstring(L, 1);
        const bool ret = SteamUserStats()->ClearAchievement(pchName);
        lua_pushboolean(L, ret);
        return 1;
    };
    static int GetAchievement                     (lua_State* L)
    {
        const char *pchName = luaL_checkstring(L, 1);
        bool bAchieved = false;
        const bool ret = SteamUserStats()->GetAchievement(pchName, &bAchieved);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, bAchieved);
        return 2;
    };
    static int GetAchievementAchievedPercent      (lua_State* L)
    {
        const char *pchName = luaL_checkstring(L, 1);
        float flPercent = 0.0f;
        const bool ret = SteamUserStats()->GetAchievementAchievedPercent(pchName, &flPercent);
        lua_pushboolean(L, ret);
        lua_pushnumber(L, flPercent);
        return 2;
    };
    static int GetAchievementAndUnlockTime        (lua_State* L)
    {
        const char *pchName = luaL_checkstring(L, 1);
        bool bAchieved = false;
        uint32 unUnlockTime = 0;
        const bool ret = SteamUserStats()->GetAchievementAndUnlockTime(pchName, &bAchieved, &unUnlockTime);
        lua_pushboolean(L, ret);
        lua_pushboolean(L, bAchieved);
        lua_push_uint32(L, unUnlockTime);
        return 3;
    };
    static int GetAchievementDisplayAttribute     (lua_State* L)
    {
        const char* pchName = luaL_checkstring(L, 1);
        const char* pchKey = luaL_checkstring(L, 2);
        const char* ret = SteamUserStats()->GetAchievementDisplayAttribute(pchName, pchKey);
        lua_pushstring(L, ret);
        return 1;
    };
    static int GetAchievementIcon                 (lua_State* L) // !
    {
        return 0;
    };
    static int GetAchievementName                 (lua_State* L)
    {
        const uint32 iAchievement = lua_to_uint32(L, 1);
        const char* ret = SteamUserStats()->GetAchievementName(iAchievement);
        lua_pushstring(L, ret);
        return 1;
    };
    static int GetMostAchievedAchievementInfo     (lua_State* L) // !
    {
        return 0;
    };
    static int GetNextMostAchievedAchievementInfo (lua_State* L) // !
    {
        return 0;
    };
    static int GetNumAchievements                 (lua_State* L)
    {
        const uint32 ret = SteamUserStats()->GetNumAchievements();
        lua_push_uint32(L, ret);
        return 1;
    };
    static int GetUserAchievement                 (lua_State* L) // !
    {
        return 0;
    };
    static int GetUserAchievementAndUnlockTime    (lua_State* L) // !
    {
        return 0;
    };
    static int IndicateAchievementProgress        (lua_State* L)
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
    static int SetAchievement                     (lua_State* L)
    {
        const char *pchName = luaL_checkstring(L, 1);
        const bool ret = SteamUserStats()->SetAchievement(pchName);
        lua_pushboolean(L, ret);
        return 1;
    };
    
    static int GetGlobalStat       (lua_State* L) // !
    {
        return 0;
    };
    static int GetGlobalStatHistory(lua_State* L) // !
    {
        return 0;
    };
    static int GetStat             (lua_State* L)
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
    static int GetUserStat         (lua_State* L) // !
    {
        return 0;
    };
    static int RequestGlobalStats  (lua_State* L) // !
    {
        return 0;
    };
    static int RequestUserStats    (lua_State* L) // !
    {
        return 0;
    };
    static int SetStat             (lua_State* L)
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
    static int UpdateAvgRateStat   (lua_State* L)
    {
        const char* pchName = luaL_checkstring(L, 1);
        const float flCountThisSession = (float)luaL_checknumber(L, 2);
        const double dSessionLength = luaL_checknumber(L, 3);
        const bool ret = SteamUserStats()->UpdateAvgRateStat(pchName, flCountThisSession, dSessionLength);
        lua_pushboolean(L, ret);
        return 1;
    };
    
    static int AttachLeaderboardUGC              (lua_State* L) // !
    {
        return 0;
    };
    static int DownloadLeaderboardEntries        (lua_State* L) // !
    {
        return 0;
    };
    static int DownloadLeaderboardEntriesForUsers(lua_State* L) // !
    {
        return 0;
    };
    static int FindLeaderboard                   (lua_State* L) // !
    {
        return 0;
    };
    static int FindOrCreateLeaderboard           (lua_State* L) // !
    {
        return 0;
    };
    static int GetDownloadedLeaderboardEntry     (lua_State* L) // !
    {
        return 0;
    };
    static int GetLeaderboardDisplayType         (lua_State* L) // !
    {
        return 0;
    };
    static int GetLeaderboardEntryCount          (lua_State* L) // !
    {
        return 0;
    };
    static int GetLeaderboardName                (lua_State* L) // !
    {
        return 0;
    };
    static int GetLeaderboardSortMethod          (lua_State* L) // !
    {
        return 0;
    };
    static int UploadLeaderboardScore            (lua_State* L) // !
    {
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
        return 0;
    };
};
