#include "AppFrame.h"
#include "LuaWrapper\LuaWrapper.hpp"

namespace LuaSTGPlus
{
    namespace LuaWrapper {
        void DInputWrapper::Register(lua_State* L)LNOEXCEPT
        {
            #define LDI() native::DirectInput* self = LuaSTGPlus::AppFrame::GetInstance().GetDInput()
            
            struct Wrapper
            {
                static int count(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t cnt = self->count();
                        lua_pushinteger(L, (lua_Integer)cnt);
                    }
                    else
                    {
                        lua_pushinteger(L, 0);
                    }
                    return 1;
                };
                static int refresh(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t cnt = self->refresh();
                        lua_pushinteger(L, (lua_Integer)cnt);
                    }
                    else
                    {
                        lua_pushinteger(L, 0);
                    }
                    return 1;
                };
                static int update(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        self->update();
                    }
                    return 0;
                };
                static int reset(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        self->reset();
                    }
                    return 0;
                };
                static int getAxisRange(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t index = (uint32_t)(luaL_checkinteger(L, 1) - 1);
                        native::DirectInput::AxisRange range;
                        if (self->getAxisRange(index, &range))
                        {
                            lua_createtable(L, 0, 12);
                            
                            #define setint(_K) lua_pushstring(L, #_K); lua_pushinteger(L, (lua_Integer)range._K); lua_settable(L, -3);
                            setint(XMin); setint(YMin); setint(ZMin);
                            setint(XMax); setint(YMax); setint(ZMax);
                            setint(RxMin); setint(RyMin); setint(RzMin);
                            setint(RxMax); setint(RyMax); setint(RzMax);
                            #undef setint
                            
                            return 1;
                        }
                    }
                    return 0;
                };
                static int getRawState(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t index = (uint32_t)(luaL_checkinteger(L, 1) - 1);
                        native::DirectInput::RawState state;
                        if (self->getRawState(index, &state))
                        {
                            lua_createtable(L, 0, 9);
                            #define setint(_K) lua_pushstring(L, #_K); lua_pushinteger(L, (lua_Integer)state._K); lua_settable(L, -3);
                            setint(lX);
                            setint(lY);
                            setint(lZ);
                            setint(lRx);
                            setint(lRy);
                            setint(lRz);
                            #undef setint
                            
                            lua_pushstring(L, "rglSlider");
                            lua_createtable(L, 2, 0);
                            for (int i = 0; i < 2; i += 1)
                            {
                                lua_pushinteger(L, i + 1);
                                lua_pushinteger(L, (lua_Integer)state.rglSlider[i]);
                                lua_settable(L, -3);
                            }
                            lua_settable(L, -3);
                            
                            lua_pushstring(L, "rgdwPOV");
                            lua_createtable(L, 4, 0);
                            for (int i = 0; i < 4; i += 1)
                            {
                                lua_pushinteger(L, i + 1);
                                lua_pushinteger(L, (lua_Integer)(state.rgdwPOV[i] & 0xFFFF));
                                lua_settable(L, -3);
                            }
                            lua_settable(L, -3);
                            
                            lua_pushstring(L, "rgbButtons");
                            lua_createtable(L, 32, 0);
                            for (int i = 0; i < 32; i += 1)
                            {
                                lua_pushinteger(L, i + 1);
                                lua_pushboolean(L, state.rgbButtons[i] != 0);
                                lua_settable(L, -3);
                            }
                            lua_settable(L, -3);
                            
                            return 1;
                        }
                    }
                    return 0;
                };
                static int getState(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t index = (uint32_t)(luaL_checkinteger(L, 1) - 1);
                        native::DirectInput::State state;
                        if (self->getState(index, &state))
                        {
                            lua_createtable(L, 0, 7);
                            
                            #define setint(_K) lua_pushstring(L, #_K); lua_pushinteger(L, (lua_Integer)state._K); lua_settable(L, -3);
                            setint(wButtons);
                            setint(bLeftTrigger);
                            setint(bRightTrigger);
                            setint(sThumbLX);
                            setint(sThumbLY);
                            setint(sThumbRX);
                            setint(sThumbRY);
                            #undef setint
                            
                            return 1;
                        }
                    }
                    return 0;
                };
                static int getDeviceName(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t index = (uint32_t)(luaL_checkinteger(L, 1) - 1);
                        const wchar_t* name = self->getDeviceName(index);
                        if (name)
                        {
                            try
                            {
                                std::string utf8name = fcyStringHelper::WideCharToMultiByte(name);
                                lua_pushstring(L, utf8name.c_str());
                                return 1;
                            }
                            catch (...) {}
                        }
                    }
                    return 0;
                };
                static int getProductName(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t index = (uint32_t)(luaL_checkinteger(L, 1) - 1);
                        const wchar_t* name = self->getProductName(index);
                        if (name)
                        {
                            try
                            {
                                std::string utf8name = fcyStringHelper::WideCharToMultiByte(name);
                                lua_pushstring(L, utf8name.c_str());
                                return 1;
                            }
                            catch (...) {}
                        }
                    }
                    return 0;
                };
                static int isXInputDevice(lua_State* L)
                {
                    LDI();
                    if (self)
                    {
                        uint32_t index = (uint32_t)(luaL_checkinteger(L, 1) - 1);
                        lua_pushboolean(L, self->isXInputDevice(index));
                        return 1;
                    }
                    lua_pushboolean(L, false);
                    return 1;
                };
            };
            
            const luaL_Reg lib[] = {
                {"count", &Wrapper::count},
                {"refresh", &Wrapper::refresh},
                {"update", &Wrapper::update},
                {"reset", &Wrapper::reset},
                {"getAxisRange", &Wrapper::getAxisRange},
                {"getRawState", &Wrapper::getRawState},
                {"getState", &Wrapper::getState},
                {"getDeviceName", &Wrapper::getDeviceName},
                {"getProductName", &Wrapper::getProductName},
                {"isXInputDevice", &Wrapper::isXInputDevice},
                {NULL, NULL},
            };
            
            lua_pushstring(L, "DirectInput"); // ??? lstg "DirectInput"
            lua_newtable(L);                  // ??? lstg "DirectInput" lib
            luaL_register(L, NULL, lib);      // ??? lstg "DirectInput" lib
            lua_settable(L, -3);              // ??? lstg
            luaL_register(L, "dinput", lib);  // ??? lstg lib
            lua_pop(L, 1);                    // ??? lstg
        }
    };
};
