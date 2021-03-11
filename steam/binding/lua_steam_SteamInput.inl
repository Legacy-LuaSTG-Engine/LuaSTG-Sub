#pragma once

struct xSteamInput
{
    static int Init(lua_State* L)
    {
        const bool ret = SteamInput()->Init();
        lua_pushboolean(L, ret);
        return 1;
    };
    static int Shutdown(lua_State* L)
    {
        (void) SteamInput()->Shutdown(); // always return true, ignore it
        return 0;
    };
    static int RunFrame(lua_State* L)
    {
        SteamInput()->RunFrame();
        return 0;
    };
    
    static int GetConnectedControllers(lua_State* L)
    {
        InputHandle_t handlesOut[STEAM_INPUT_MAX_COUNT] = {};
        const int count = SteamInput()->GetConnectedControllers(handlesOut);
        if (lua_istable(L, 1))
        {
            for (int idx = 0; idx < count; idx++)
            {
                lua_pushinteger(L, idx + 1);
                lua_push_InputHandle_t(L, handlesOut[idx]);
                lua_settable(L, 1);
            }
            lua_pushinteger(L, count);
        }
        else
        {
            lua_createtable(L, count, 0);
            for (int idx = 0; idx < count; idx++)
            {
                lua_push_InputHandle_t(L, handlesOut[idx]);
                lua_rawseti(L, -2, idx + 1);
            }
        }
        return 1;
    };
    static int GetControllerForGamepadIndex(lua_State* L)
    {
        const int nIndex = luaL_checkinteger(L, 1);
        const InputHandle_t ret = SteamInput()->GetControllerForGamepadIndex(nIndex);
        lua_push_InputHandle_t(L, ret);
        return 1;
    };
    static int GetGamepadIndexForController(lua_State* L)
    {
        const InputHandle_t ulinputHandle = lua_to_InputHandle_t(L, 1);
        const int ret = SteamInput()->GetGamepadIndexForController(ulinputHandle);
        lua_pushinteger(L, ret);
        return 1;
    };
    static int GetInputTypeForHandle(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const ESteamInputType ret = SteamInput()->GetInputTypeForHandle(inputHandle);
        lua_pushinteger(L, (lua_Integer)ret);
        switch (ret)
        {
        case k_ESteamInputType_Unknown:
            lua_pushstring(L, "Unknown controller"); break;
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
        case k_ESteamInputType_AppleMFiController:
        case k_ESteamInputType_AndroidController:
        case k_ESteamInputType_SwitchJoyConPair:
        case k_ESteamInputType_SwitchJoyConSingle:
            lua_pushstring(L, "Unused controller"); break;
        case k_ESteamInputType_SwitchProController:
            lua_pushstring(L, "Switch Pro controller"); break;
        case k_ESteamInputType_MobileTouch:
            lua_pushstring(L, "Mobile Touch controller"); break;
        case k_ESteamInputType_PS3Controller:
            lua_pushstring(L, "PS4 controller"); break;
        default:
            lua_pushstring(L, "Error"); break;
        }
        return 2;
    };
    
    static int GetActionSetHandle(lua_State* L)
    {
        const char* pszActionSetName = luaL_checkstring(L, 1);
        const InputActionSetHandle_t ret = SteamInput()->GetActionSetHandle(pszActionSetName);
        lua_push_InputActionSetHandle_t(L, ret);
        return 1;
    };
    static int ActivateActionSet(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputActionSetHandle_t actionSetHandle = lua_to_InputActionSetHandle_t(L, 2);
        SteamInput()->ActivateActionSet(inputHandle, actionSetHandle);
        return 0;
    };
    static int GetCurrentActionSet(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputActionSetHandle_t ret = SteamInput()->GetCurrentActionSet(inputHandle);
        lua_push_InputActionSetHandle_t(L, ret);
        return 1;
    };
    static int ActivateActionSetLayer(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputActionSetHandle_t actionSetLayerHandle = lua_to_InputActionSetHandle_t(L, 2);
        SteamInput()->ActivateActionSetLayer(inputHandle, actionSetLayerHandle);
        return 0;
    };
    static int GetActiveActionSetLayers(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        InputActionSetHandle_t handlesOut[STEAM_INPUT_MAX_ACTIVE_LAYERS] = {};
        const int count = SteamInput()->GetActiveActionSetLayers(inputHandle, handlesOut);
        if (lua_istable(L, 2))
        {
            for (int idx = 0; idx < count; idx++)
            {
                lua_pushinteger(L, idx + 1);
                lua_push_InputActionSetHandle_t(L, handlesOut[idx]);
                lua_settable(L, 2);
            }
            lua_pushinteger(L, count);
        }
        else
        {
            lua_createtable(L, count, 0);
            for (int idx = 0; idx < count; idx++)
            {
                lua_push_InputActionSetHandle_t(L, handlesOut[idx]);
                lua_rawseti(L, -2, idx + 1);
            }
        }
        return 1;
    };
    static int DeactivateActionSetLayer(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputActionSetHandle_t actionSetLayerHandle = lua_to_InputActionSetHandle_t(L, 2);
        SteamInput()->DeactivateActionSetLayer(inputHandle, actionSetLayerHandle);
        return 0;
    };
    static int DeactivateAllActionSetLayers(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        SteamInput()->DeactivateAllActionSetLayers(inputHandle);
        return 0;
    };
    
    static int GetAnalogActionHandle(lua_State* L)
    {
        const char* pszActionName = luaL_checkstring(L, 1);
        const InputAnalogActionHandle_t ret = SteamInput()->GetAnalogActionHandle(pszActionName);
        lua_push_InputAnalogActionHandle_t(L, ret);
        return 1;
    };
    static int GetDigitalActionHandle(lua_State* L)
    {
        const char* pszActionName = luaL_checkstring(L, 1);
        const InputDigitalActionHandle_t ret = SteamInput()->GetDigitalActionHandle(pszActionName);
        lua_push_InputDigitalActionHandle_t(L, ret);
        return 1;
    };
    static int GetAnalogActionData(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputAnalogActionHandle_t analogActionHandle = lua_to_InputAnalogActionHandle_t(L, 2);
        const InputAnalogActionData_t ret = SteamInput()->GetAnalogActionData(inputHandle, analogActionHandle);
        if (lua_istable(L, 3))
        {
            lua_pushinteger(L, (lua_Integer)ret.eMode);
            lua_setfield(L, 3, "eMode");
            lua_pushnumber(L, ret.x);
            lua_setfield(L, 3, "x");
            lua_pushnumber(L, ret.y);
            lua_setfield(L, 3, "y");
            lua_pushboolean(L, ret.bActive);
            lua_setfield(L, 3, "bActive");
            return 0;
        }
        else
        {
            lua_createtable(L, 0, 4);
            lua_pushinteger(L, (lua_Integer)ret.eMode);
            lua_setfield(L, -2, "eMode");
            lua_pushnumber(L, ret.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, ret.y);
            lua_setfield(L, -2, "y");
            lua_pushboolean(L, ret.bActive);
            lua_setfield(L, -2, "bActive");
            return 1;
        }
    };
    static int GetDigitalActionData(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputDigitalActionHandle_t digitalActionHandle = lua_to_InputDigitalActionHandle_t(L, 2);
        const InputDigitalActionData_t ret = SteamInput()->GetDigitalActionData(inputHandle, digitalActionHandle);
        if (lua_istable(L, 3))
        {
            lua_pushboolean(L, ret.bState);
            lua_setfield(L, 3, "bState");
            lua_pushboolean(L, ret.bActive);
            lua_setfield(L, 3, "bActive");
            return 0;
        }
        else
        {
            lua_createtable(L, 0, 4);
            lua_pushboolean(L, ret.bState);
            lua_setfield(L, -2, "bState");
            lua_pushboolean(L, ret.bActive);
            lua_setfield(L, -2, "bActive");
            return 1;
        }
    };
    static int GetMotionData(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputMotionData_t ret = SteamInput()->GetMotionData(inputHandle);
        if (lua_istable(L, 2))
        {
            lua_pushnumber(L, ret.rotQuatX);
            lua_setfield(L, 2, "rotQuatX");
            lua_pushnumber(L, ret.rotQuatY);
            lua_setfield(L, 2, "rotQuatY");
            lua_pushnumber(L, ret.rotQuatZ);
            lua_setfield(L, 2, "rotQuatZ");
            lua_pushnumber(L, ret.rotQuatW);
            lua_setfield(L, 2, "rotQuatW");
            
            lua_pushnumber(L, ret.posAccelX);
            lua_setfield(L, 2, "posAccelX");
            lua_pushnumber(L, ret.posAccelY);
            lua_setfield(L, 2, "posAccelY");
            lua_pushnumber(L, ret.posAccelZ);
            lua_setfield(L, 2, "posAccelZ");
            
            lua_pushnumber(L, ret.rotVelX);
            lua_setfield(L, 2, "rotVelX");
            lua_pushnumber(L, ret.rotVelY);
            lua_setfield(L, 2, "rotVelY");
            lua_pushnumber(L, ret.rotVelZ);
            lua_setfield(L, 2, "rotVelZ");
            
            return 1;
        }
        else
        {
            lua_createtable(L, 0, 10);
            
            lua_pushnumber(L, ret.rotQuatX);
            lua_setfield(L, -2, "rotQuatX");
            lua_pushnumber(L, ret.rotQuatY);
            lua_setfield(L, -2, "rotQuatY");
            lua_pushnumber(L, ret.rotQuatZ);
            lua_setfield(L, -2, "rotQuatZ");
            lua_pushnumber(L, ret.rotQuatW);
            lua_setfield(L, -2, "rotQuatW");
            
            lua_pushnumber(L, ret.posAccelX);
            lua_setfield(L, -2, "posAccelX");
            lua_pushnumber(L, ret.posAccelY);
            lua_setfield(L, -2, "posAccelY");
            lua_pushnumber(L, ret.posAccelZ);
            lua_setfield(L, -2, "posAccelZ");
            
            lua_pushnumber(L, ret.rotVelX);
            lua_setfield(L, -2, "rotVelX");
            lua_pushnumber(L, ret.rotVelY);
            lua_setfield(L, -2, "rotVelY");
            lua_pushnumber(L, ret.rotVelZ);
            lua_setfield(L, -2, "rotVelZ");
            
            return 1;
        }
    };
    
    static int GetAnalogActionOrigins(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputActionSetHandle_t actionSetHandle = lua_to_InputActionSetHandle_t(L, 2);
        const InputAnalogActionHandle_t analogActionHandle = lua_to_InputAnalogActionHandle_t(L, 3);
        EInputActionOrigin originsOut[STEAM_INPUT_MAX_ORIGINS] = {};
        const int count = SteamInput()->GetAnalogActionOrigins(inputHandle, actionSetHandle, analogActionHandle, originsOut);
        if (lua_istable(L, 4))
        {
            for (int idx = 0; idx < count; idx++)
            {
                lua_pushinteger(L, idx + 1);
                lua_pushinteger(L, (lua_Integer)originsOut[idx]);
                lua_settable(L, 4);
            }
            lua_pushinteger(L, count);
        }
        else
        {
            lua_createtable(L, count, 0);
            for (int idx = 0; idx < count; idx++)
            {
                lua_pushinteger(L, (lua_Integer)originsOut[idx]);
                lua_rawseti(L, -2, idx + 1);
            }
        }
        return 1;
    };
    static int GetDigitalActionOrigins(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputActionSetHandle_t actionSetHandle = lua_to_InputActionSetHandle_t(L, 2);
        const InputDigitalActionHandle_t digitalActionHandle = lua_to_InputDigitalActionHandle_t(L, 3);
        EInputActionOrigin originsOut[STEAM_INPUT_MAX_ORIGINS] = {};
        const int count = SteamInput()->GetDigitalActionOrigins(inputHandle, actionSetHandle, digitalActionHandle, originsOut);
        if (lua_istable(L, 4))
        {
            for (int idx = 0; idx < count; idx++)
            {
                lua_pushinteger(L, idx + 1);
                lua_pushinteger(L, (lua_Integer)originsOut[idx]);
                lua_settable(L, 4);
            }
            lua_pushinteger(L, count);
        }
        else
        {
            lua_createtable(L, count, 0);
            for (int idx = 0; idx < count; idx++)
            {
                lua_pushinteger(L, (lua_Integer)originsOut[idx]);
                lua_rawseti(L, -2, idx + 1);
            }
        }
        return 1;
    };
    static int GetGlyphForActionOrigin(lua_State* L)
    {
        const EInputActionOrigin eOrigin = (EInputActionOrigin)luaL_checkinteger(L, 1);
        const char* ret = SteamInput()->GetGlyphForActionOrigin(eOrigin);
        lua_pushstring(L, ret);
        return 1;
    };
    static int GetStringForActionOrigin(lua_State* L)
    {
        const EInputActionOrigin eOrigin = (EInputActionOrigin)luaL_checkinteger(L, 1);
        const char* ret = SteamInput()->GetStringForActionOrigin(eOrigin);
        lua_pushstring(L, ret);
        return 1;
    };
    static int TranslateActionOrigin(lua_State* L)
    {
        const ESteamInputType eDestinationInputType = (ESteamInputType)luaL_checkinteger(L, 1);
        const EInputActionOrigin eSourceOrigin = (EInputActionOrigin)luaL_checkinteger(L, 2);
        const EInputActionOrigin ret = SteamInput()->TranslateActionOrigin(eDestinationInputType, eSourceOrigin);
        lua_pushinteger(L, (lua_Integer)ret);
        return 1;
    };
    static int GetActionOriginFromXboxOrigin(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const EXboxOrigin eOrigin = (EXboxOrigin)luaL_checkinteger(L, 2);
        const EInputActionOrigin ret = SteamInput()->GetActionOriginFromXboxOrigin(inputHandle, eOrigin);
        lua_pushinteger(L, (lua_Integer)ret);
        return 1;
    };
    
    static int SetLEDColor(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const uint8 nColorR = (uint8)luaL_checkinteger(L, 2);
        const uint8 nColorG = (uint8)luaL_checkinteger(L, 3);
        const uint8 nColorB = (uint8)luaL_checkinteger(L, 4);
        const unsigned int nFlags = lua_to_uint(L, 5);
        SteamInput()->SetLEDColor(inputHandle, nColorR, nColorG, nColorB, nFlags);
        return 0;
    };
    static int StopAnalogActionMomentum(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const InputAnalogActionHandle_t eAction = lua_to_InputAnalogActionHandle_t(L, 2);
        SteamInput()->StopAnalogActionMomentum(inputHandle, eAction);
        return 0;
    };
    static int TriggerHapticPulse(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const ESteamControllerPad eTargetPad = (ESteamControllerPad)luaL_checkinteger(L, 2);
        const unsigned short usDurationMicroSec = (unsigned short)luaL_checkinteger(L, 3);
        SteamInput()->TriggerHapticPulse(inputHandle, eTargetPad, usDurationMicroSec);
        return 0;
    };
    static int TriggerRepeatedHapticPulse(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const ESteamControllerPad eTargetPad = (ESteamControllerPad)luaL_checkinteger(L, 2);
        const unsigned short usDurationMicroSec = (unsigned short)luaL_checkinteger(L, 3);
        const unsigned short usOffMicroSec      = (unsigned short)luaL_checkinteger(L, 4);
        const unsigned short unRepeat           = (unsigned short)luaL_checkinteger(L, 5);
        const unsigned int nFlags = lua_to_uint(L, 6);
        SteamInput()->TriggerRepeatedHapticPulse(inputHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags);
        return 0;
    };
    static int TriggerVibration(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const unsigned short usLeftSpeed  = (unsigned short)luaL_checkinteger(L, 2);
        const unsigned short usRightSpeed = (unsigned short)luaL_checkinteger(L, 3);
        SteamInput()->TriggerVibration(inputHandle, usLeftSpeed, usRightSpeed);
        return 0;
    };
    
    static int ShowBindingPanel(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const bool ret = SteamInput()->ShowBindingPanel(inputHandle);
        lua_pushboolean(L, ret);
        return 1;
    };
    
    static int GetDeviceBindingRevision(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        int Major = 0;
        int Minor = 0;
        const bool ret = SteamInput()->GetDeviceBindingRevision(inputHandle, &Major, &Minor);
        lua_pushboolean(L, ret);
        lua_pushinteger(L, Major);
        lua_pushinteger(L, Minor);
        return 3;
    };
    static int GetRemotePlaySessionID(lua_State* L)
    {
        const InputHandle_t inputHandle = lua_to_InputHandle_t(L, 1);
        const uint32_t ret = SteamInput()->GetRemotePlaySessionID(inputHandle);
        lua_push_uint32(L, ret);
        return 1;
    };
    
    static int xRegister(lua_State* L)
    {
        static const luaL_Reg lib[] = {
            xfbinding(Init),
            xfbinding(Shutdown),
            xfbinding(RunFrame),
            
            xfbinding(GetConnectedControllers),
            xfbinding(GetControllerForGamepadIndex),
            xfbinding(GetGamepadIndexForController),
            xfbinding(GetInputTypeForHandle),
            
            xfbinding(GetActionSetHandle),
            xfbinding(ActivateActionSet),
            xfbinding(GetCurrentActionSet),
            xfbinding(ActivateActionSetLayer),
            xfbinding(GetActiveActionSetLayers),
            xfbinding(DeactivateActionSetLayer),
            xfbinding(DeactivateAllActionSetLayers),
            
            xfbinding(GetAnalogActionHandle),
            xfbinding(GetDigitalActionHandle),
            xfbinding(GetAnalogActionData),
            xfbinding(GetDigitalActionData),
            xfbinding(GetMotionData),
            
            xfbinding(GetAnalogActionOrigins),
            xfbinding(GetDigitalActionOrigins),
            xfbinding(GetGlyphForActionOrigin),
            xfbinding(GetStringForActionOrigin),
            xfbinding(TranslateActionOrigin),
            xfbinding(GetActionOriginFromXboxOrigin),
            
            xfbinding(SetLEDColor),
            xfbinding(StopAnalogActionMomentum),
            xfbinding(TriggerHapticPulse),
            xfbinding(TriggerRepeatedHapticPulse),
            xfbinding(TriggerVibration),
            
            xfbinding(ShowBindingPanel),
            
            xfbinding(GetDeviceBindingRevision),
            xfbinding(GetRemotePlaySessionID),
            
            {NULL, NULL},
        };
        lua_pushstring(L, "SteamInput");
        lua_createtable(L, 0, 33);
        luaL_register(L, NULL, lib);
        lua_settable(L, -3);
        return 0;
    };
};
