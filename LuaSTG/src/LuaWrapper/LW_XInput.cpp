#include "LuaWrapper\LuaWrapper.hpp"
#include "E2DXInput.hpp"
#include "AppFrame.h"

using namespace std;
using namespace LuaSTGPlus;

void XInputManagerWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
		static int GetKeyState(lua_State* L)
		{
			int index = luaL_checkinteger(L, 1);
			int vkey = luaL_checkinteger(L, 2);
			lua_pushboolean(L, LXINPUT.GetKeyState(index, vkey));
			return 1;
		}
		static int GetTriggerState(lua_State* L) {
			int index = luaL_checkinteger(L, 1);
			lua_Integer left = LXINPUT.GetTriggerStateL(index);
			lua_Integer right = LXINPUT.GetTriggerStateR(index);
			lua_pushinteger(L,left);
			lua_pushinteger(L, right);
			return 2;
		}
		static int GetThumbState(lua_State* L) {
			int index = luaL_checkinteger(L, 1);
			lua_Integer LX = LXINPUT.GetThumbStateLX(index);
			lua_Integer LY = LXINPUT.GetThumbStateLY(index);
			lua_Integer RX = LXINPUT.GetThumbStateRX(index);
			lua_Integer RY = LXINPUT.GetThumbStateRY(index);
			lua_pushinteger(L, LX);
			lua_pushinteger(L, LY);
			lua_pushinteger(L, RX);
			lua_pushinteger(L, RY);
			return 4;
		}
		static int SetMotorSpeed(lua_State* L) {
			int index = luaL_checkinteger(L, 1);
			int _L = luaL_checkinteger(L, 2);
			int _H = luaL_checkinteger(L, 3);
			bool ret = LXINPUT.SetMotorSpeed(index, _L, _H);
			lua_pushboolean(L, ret);
			return 1;
		}
		static int GetMotorSpeed(lua_State* L) {
			int index = luaL_checkinteger(L, 1);
			lua_Integer _L = LXINPUT.GetMotorSpeedLow(index);
			lua_Integer _H = LXINPUT.GetMotorSpeedHigh(index);
			lua_pushinteger(L, _L);
			lua_pushinteger(L, _H);
			lua_pushboolean(L, (_L >= 0) && (_H >= 0));//Ê§°ÜÎªfalse
			return 3;
		}
		static int Refresh(lua_State* L) {
			lua_pushinteger(L, LXINPUT.Refresh());
			return 1;
		}
		static int Update(lua_State* L) {
			LXINPUT.Update();
			return 0;
		}
		static int GetDeviceCount(lua_State* L) {
			lua_pushinteger(L, LXINPUT.GetDeviceCount());
			return 1;
		}
	};

	luaL_Reg tMethods[] =
	{
		{ "GetKeyState", &WrapperImplement::GetKeyState },
		{ "GetTriggerState", &WrapperImplement::GetTriggerState },
		{ "GetThumbState", &WrapperImplement::GetThumbState },
		{ "SetMotorSpeed", &WrapperImplement::SetMotorSpeed },
		{ "GetMotorSpeed", &WrapperImplement::GetMotorSpeed },
		{ "Refresh", &WrapperImplement::Refresh },
		{ "Update", &WrapperImplement::Update },
		{ "GetDeviceCount", &WrapperImplement::GetDeviceCount },
		{ NULL, NULL }
	};
	
	lua_getglobal(L, "lstg"); // ??? t 
	lua_newtable(L); // ??? t t
	::luaL_register(L, NULL, tMethods); // ??? t t 
	lua_setfield(L, -2, "XInputManager"); // ??? t 
	lua_pop(L, 1); // ??? 
}
