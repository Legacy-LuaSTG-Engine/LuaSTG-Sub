#include "LuaWrapper/LW_SteamAPI.h"
#include "lua.hpp"

#ifdef USING_STEAM_API
#pragma comment(lib, "steam_api.lib")
#include "steam_api.h"

#pragma region input
int _input_handle_to_lua_value(lua_State* L, uint64_t v) {
	uint32_t vl = 0, vh = 0;
	vl = 0x00000000FFFFFFFF & v;
	vh = (0xFFFFFFFF00000000 & v) >> 32;
	lua_createtable(L, 2, 0);          // ??? t
	lua_pushinteger(L, 1);             // ??? t 1
	lua_pushnumber(L, (lua_Number)vl); // ??? t 1 vl
	lua_settable(L, -3);               // ??? t
	lua_pushinteger(L, 2);             // ??? t 2
	lua_pushnumber(L, (lua_Number)vh); // ??? t 2 vh
	lua_settable(L, -3);               // ??? t
	return 1;
}

uint64_t _lua_value_to_input_handle(lua_State* L, int n) {
	uint32_t vl = 0, vh = 0;
	lua_pushvalue(L, n);                // ??? t
	lua_pushinteger(L, 1);              // ??? t 1
	lua_gettable(L, -2);                // ??? t vl
	vl = (uint32_t)lua_tonumber(L, -1);
	lua_pop(L, 1);                      // ??? t
	lua_pushinteger(L, 2);              // ??? t 2
	lua_gettable(L, -2);                // ??? t vh
	vh = (uint32_t)lua_tonumber(L, -1);
	lua_pop(L, 2);                      // ???
	return (uint64_t)vl + ((uint64_t)vh << 32);
}

// input

static int input_Init(lua_State* L) {
	lua_pushboolean(L, SteamInput()->Init());
	return 1;
}

static int input_Shutdown(lua_State* L) {
	lua_pushboolean(L, SteamInput()->Shutdown());
	return 1;
}

static int input_RunFrame(lua_State* L) {
	SteamInput()->RunFrame();
	return 0;
}

static int input_GetConnectedControllers(lua_State* L) {
	InputHandle_t handles[STEAM_INPUT_MAX_COUNT];
	int count = SteamInput()->GetConnectedControllers(handles);
	
	lua_createtable(L, count, 0);                  // t
	for (int i = 0; i < count; i++) {
		lua_pushinteger(L, i + 1);                 // t i
		_input_handle_to_lua_value(L, handles[i]); // t i h
		lua_settable(L, 1);                        // t
	}

	return 1;
}

// action set

static int input_GetActionSetHandle(lua_State* L) {
	_input_handle_to_lua_value(L,
		SteamInput()->GetActionSetHandle(luaL_checkstring(L, 1)));
	return 1;
}

static int input_ActivateActionSet(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	InputActionSetHandle_t set = _lua_value_to_input_handle(L, 2);
	SteamInput()->ActivateActionSet(handle, set);
	return 0;
}

static int input_GetCurrentActionSet(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	_input_handle_to_lua_value(L,
		SteamInput()->GetCurrentActionSet(handle));
	return 1;
}

static int input_ActivateActionSetLayer(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	InputActionSetHandle_t set = _lua_value_to_input_handle(L, 2);
	SteamInput()->ActivateActionSetLayer(handle, set);
	return 0;
}

static int input_DeactivateActionSetLayer(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	InputActionSetHandle_t set = _lua_value_to_input_handle(L, 2);
	SteamInput()->DeactivateActionSetLayer(handle, set);
	return 0;
}

static int input_DeactivateAllActionSetLayers(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	SteamInput()->DeactivateAllActionSetLayers(handle);
	return 0;
}

static int input_GetActiveActionSetLayers(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	InputActionSetHandle_t set[STEAM_INPUT_MAX_COUNT];
	int count = SteamInput()->GetActiveActionSetLayers(handle, set);

	lua_createtable(L, count, 0);              // t
	for (int i = 0; i < count; i++) {
		lua_pushinteger(L, i + 1);             // t i
		_input_handle_to_lua_value(L, set[i]); // t i h
		lua_settable(L, 1);                    // t
	}

	return 1;
}

// action

static int input_GetDigitalActionHandle(lua_State* L) {
	_input_handle_to_lua_value(L,
		SteamInput()->GetDigitalActionHandle(luaL_checkstring(L, 1)));
	return 1;
}

static int input_GetDigitalActionData(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	InputDigitalActionHandle_t action = _lua_value_to_input_handle(L, 2);
	InputDigitalActionData_t data = SteamInput()->GetDigitalActionData(handle, action);
	lua_pushboolean(L, data.bState);
	lua_pushboolean(L, data.bActive);
	return 2;
}

static int input_GetAnalogActionHandle(lua_State* L) {
	_input_handle_to_lua_value(L,
		SteamInput()->GetAnalogActionHandle(luaL_checkstring(L, 1)));
	return 1;
}

static int input_GetAnalogActionData(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	InputAnalogActionHandle_t action = _lua_value_to_input_handle(L, 2);
	InputAnalogActionData_t data = SteamInput()->GetAnalogActionData(handle, action);
	lua_pushnumber(L, (lua_Number)data.x);
	lua_pushnumber(L, (lua_Number)data.y);
	lua_pushboolean(L, data.bActive);
	return 3;
}

// utility

static int input_HandleEqual(lua_State* L) {
	uint64_t v1 = _lua_value_to_input_handle(L, 1);
	uint64_t v2 = _lua_value_to_input_handle(L, 2);
	lua_pushboolean(L, v1 == v2);
	return 1;
}

static int input_ShowBindingPanel(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	lua_pushboolean(L, SteamInput()->ShowBindingPanel(handle));
	return 1;
}

static int input_GetInputTypeForHandle(lua_State* L) {
	InputHandle_t handle = _lua_value_to_input_handle(L, 1);
	ESteamInputType t = SteamInput()->GetInputTypeForHandle(handle);
	lua_pushinteger(L, (lua_Integer)t);
	switch (t) {
		case k_ESteamInputType_Unknown:
			lua_pushstring(L, "Unknown type"); break;
		case k_ESteamInputType_SteamController:
			lua_pushstring(L, "Steam controller"); break;
		case k_ESteamInputType_XBox360Controller:
			lua_pushstring(L, "XBox 360 controller"); break;
		case k_ESteamInputType_XBoxOneController:
			lua_pushstring(L, "XBox One controller"); break;
		case k_ESteamInputType_GenericGamepad:
			lua_pushstring(L, "Generic controller"); break;
		case k_ESteamInputType_PS4Controller:
			lua_pushstring(L, "PS4 controller"); break;
		case k_ESteamInputType_PS3Controller:
			lua_pushstring(L, "PS4 controller"); break;
		case k_ESteamInputType_SwitchProController:
			lua_pushstring(L, "Switch Pro controller"); break;
	}
	return 2;
}

// lua map

static const char input_lib_name[] = "SteamInput";
static const int input_lib_size = 18;
static const struct luaL_Reg input_lib[input_lib_size + 1] = {
	{"Init", input_Init},
	{"Shutdown", input_Shutdown},
	{"RunFrame", input_RunFrame},
	{"GetConnectedControllers", input_GetConnectedControllers},

	{"GetActionSetHandle", input_GetActionSetHandle},
	{"ActivateActionSet", input_ActivateActionSet},
	{"GetCurrentActionSet", input_GetCurrentActionSet},
	{"ActivateActionSetLayer", input_ActivateActionSetLayer},
	{"DeactivateActionSetLayer", input_DeactivateActionSetLayer},
	{"DeactivateAllActionSetLayers", input_DeactivateAllActionSetLayers},
	{"GetActiveActionSetLayers", input_GetActiveActionSetLayers},

	{"GetDigitalActionHandle", input_GetDigitalActionHandle},
	{"GetDigitalActionData", input_GetDigitalActionData},
	{"GetAnalogActionHandle", input_GetAnalogActionHandle},
	{"GetAnalogActionData", input_GetAnalogActionData},

	{"HandleEqual", input_HandleEqual},
	{"ShowBindingPanel", input_ShowBindingPanel},
	{"GetInputTypeForHandle", input_GetInputTypeForHandle},

	{NULL, NULL},
};
#pragma endregion

#pragma region userstats

// wrapper

class SteamUserStatsWrapper
{
private:
	int64 m_iAppID; // Our current AppID
	bool m_bInitialized; // Have we called Request stats and received the callback?

public:
	SteamUserStatsWrapper() :
		m_iAppID(STEAM_APP_ID),
		m_bInitialized(false),
		m_CallbackUserStatsReceived(this, &SteamUserStatsWrapper::OnUserStatsReceived)
	{
		RequestStats();
	}

	~SteamUserStatsWrapper() {
	}

	bool RequestStats() {
		if (NULL == SteamUserStats() || NULL == SteamUser()) {
			return false;
		}
		if (!SteamUser()->BLoggedOn()) {
			return false;
		}
		return SteamUserStats()->RequestCurrentStats();
	}

	bool RequestStatsComplete() {
		return m_bInitialized;
	}

	STEAM_CALLBACK(SteamUserStatsWrapper, OnUserStatsReceived, UserStatsReceived_t,
		m_CallbackUserStatsReceived);
};

void SteamUserStatsWrapper::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		if (k_EResultOK == pCallback->m_eResult)
		{
			m_bInitialized = true;
		}
	}
}

static SteamUserStatsWrapper g_SteamUserStatsWrapper;

// achievement

static int userstats_GetAchievement(lua_State* L) {
	bool flag = false;
	bool ret = SteamUserStats()->GetAchievement(luaL_checkstring(L, 1), &flag);
	lua_pushboolean(L, ret);
	lua_pushboolean(L, flag);
	return 2;
}

static int userstats_GetAchievementAndUnlockTime(lua_State* L) {
	bool flag = false;
	uint32 time = 0u;
	bool ret = SteamUserStats()->GetAchievementAndUnlockTime(luaL_checkstring(L, 1), &flag, &time);
	lua_pushboolean(L, ret);
	lua_pushboolean(L, flag);
	lua_pushnumber(L, (lua_Number)time);
	return 3;
}

static int userstats_GetAchievementDisplayAttribute(lua_State* L) {
	const char* ret = SteamUserStats()->GetAchievementDisplayAttribute(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
	lua_pushstring(L, ret);
	return 1;
}

static int userstats_GetAchievementName(lua_State* L) {
	const char* ret = SteamUserStats()->GetAchievementName((uint32)luaL_checknumber(L, 1));
	lua_pushstring(L, ret);
	return 1;
}

static int userstats_GetNumAchievements(lua_State* L) {
	uint32 ret = SteamUserStats()->GetNumAchievements();
	lua_pushnumber(L, (lua_Number)ret);
	return 1;
}

static int userstats_SetAchievement(lua_State* L) {
	bool ret = SteamUserStats()->SetAchievement(luaL_checkstring(L, 1));
	lua_pushboolean(L, ret);
	return 1;
}

static int userstats_IndicateAchievementProgress(lua_State* L) {
	uint32 a = (uint32)luaL_checknumber(L, 2);
	uint32 b = (uint32)luaL_checknumber(L, 3);
	bool ret = SteamUserStats()->IndicateAchievementProgress(luaL_checkstring(L, 1), a, b);
	lua_pushboolean(L, ret);
	return 1;
}

// stats

static int userstats_GetStatInt(lua_State* L) {
	int32 data = 0;
	bool ret = SteamUserStats()->GetStat(luaL_checkstring(L, 1), &data);
	lua_pushboolean(L, ret);
	lua_pushnumber(L, (lua_Number)data);
	return 2;
}

static int userstats_GetStatFloat(lua_State* L) {
	float data = 0.0f;
	bool ret = SteamUserStats()->GetStat(luaL_checkstring(L, 1), &data);
	lua_pushboolean(L, ret);
	lua_pushnumber(L, (lua_Number)data);
	return 2;
}

static int userstats_SetStatInt(lua_State* L) {
	bool ret = SteamUserStats()->SetStat(luaL_checkstring(L, 1), (int32)luaL_checknumber(L, 2));
	lua_pushboolean(L, ret);
	return 1;
}

static int userstats_SetStatFloat(lua_State* L) {
	bool ret = SteamUserStats()->SetStat(luaL_checkstring(L, 1), (float)luaL_checknumber(L, 2));
	lua_pushboolean(L, ret);
	return 1;
}

// data

static int userstats_StoreStats(lua_State* L) {
	bool ret = SteamUserStats()->StoreStats();
	lua_pushboolean(L, ret);
	return 1;
}

static int userstats_RequestCurrentStats(lua_State* L) {
	bool ret = g_SteamUserStatsWrapper.RequestStats();
	lua_pushboolean(L, ret);
	return 1;
}

static int userstats_RequestCurrentStatsComplete(lua_State* L) {
	bool ret = g_SteamUserStatsWrapper.RequestStatsComplete();
	lua_pushboolean(L, ret);
	return 1;
}

// lua map

static const char userstats_lib_name[] = "SteamUserStats";
static const int userstats_lib_size = 14;
static const struct luaL_Reg userstats_lib[userstats_lib_size + 1] = {
	{"GetAchievement", userstats_GetAchievement},
	{"GetAchievementAndUnlockTime", userstats_GetAchievementAndUnlockTime},
	{"GetAchievementDisplayAttribute", userstats_GetAchievementDisplayAttribute},
	{"GetAchievementName", userstats_GetAchievementName},
	{"GetNumAchievements", userstats_GetNumAchievements},
	{"SetAchievement", userstats_SetAchievement},
	{"IndicateAchievementProgress", userstats_IndicateAchievementProgress},

	{"GetStatInt", userstats_GetStatInt},
	{"GetStatFloat", userstats_GetStatFloat},
	{"SetStatInt", userstats_SetStatInt},
	{"SetStatFloat", userstats_SetStatFloat},

	{"StoreStats", userstats_StoreStats},
	{"RequestCurrentStats", userstats_RequestCurrentStats},
	{"RequestCurrentStatsComplete", userstats_RequestCurrentStatsComplete},

	{NULL, NULL},
};
#pragma endregion

#pragma region steam

static int steam_SteamAPI_RunCallbacks(lua_State* L) {
	::SteamAPI_RunCallbacks();
	return 0;
}

static const char steam_lib_name[] = "steam";
static const int steam_lib_size = 1;
static const struct luaL_Reg steam_lib[steam_lib_size + 1] = {
	{"SteamAPI_RunCallbacks", steam_SteamAPI_RunCallbacks},

	{NULL, NULL},
};
#pragma endregion

int _luaopen_steam(lua_State* L) {
	luaL_register(L, steam_lib_name, steam_lib); // ??? lib
	lua_pushnil(L);                              // ??? lib nil
	lua_setglobal(L, steam_lib_name);            // ??? lib

	lua_createtable(L, 0, input_lib_size);       // ??? lib input
	luaL_register(L, NULL, input_lib);           // ??? lib input
	lua_setfield(L, -2, input_lib_name);         // ??? lib

	lua_createtable(L, 0, userstats_lib_size);   // ??? lib userstats
	luaL_register(L, NULL, userstats_lib);       // ??? lib userstats
	lua_setfield(L, -2, userstats_lib_name);     // ??? lib

	return 1;
}

#else

static const char steam_lib_name[] = "steam";
static const int steam_lib_size = 0;
static const struct luaL_Reg steam_lib[steam_lib_size + 1] = { {NULL, NULL} };

#endif // USING_STEAM_API

int luaopen_steam(lua_State* L) {
	#ifdef USING_STEAM_API
		return _luaopen_steam(L);
	#else // USING_STEAM_API
		luaL_register(L, steam_lib_name, steam_lib); // ??? lib
		lua_pushnil(L);                              // ??? lib nil
		lua_setglobal(L, steam_lib_name);            // ??? lib
		return 1;
	#endif // USING_STEAM_API
}
