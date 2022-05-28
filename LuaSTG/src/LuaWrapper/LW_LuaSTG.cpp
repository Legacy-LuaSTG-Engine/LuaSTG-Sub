#include "LuaWrapper/LuaWrapper.hpp"
#include "AppFrame.h"
#include "LConfig.h"

void LuaSTGPlus::BuiltInFunctionWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
		#pragma region 框架函数
		// 框架函数
		static int GetVersionNumber(lua_State* L) noexcept
		{
			lua_pushinteger(L, LUASTG_VERSION_MAJOR);
			lua_pushinteger(L, LUASTG_VERSION_MINOR);
			lua_pushinteger(L, LUASTG_VERSION_PATCH);
			return 3;
		}
		static int GetVersionName(lua_State* L) noexcept
		{
			lua_pushstring(L, LUASTG_INFO);
			return 1;
		}
		static int SetWindowed(lua_State* L)LNOEXCEPT
		{
			LAPP.SetWindowed(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetVsync(lua_State* L)LNOEXCEPT
		{
			LAPP.SetVsync(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetResolution(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) >= 4)
			{
				LAPP.SetResolution(
					(fuInt)luaL_checkinteger(L, 1),
					(fuInt)luaL_checkinteger(L, 2),
					(fuInt)luaL_checknumber(L, 3),
					(fuInt)luaL_checknumber(L, 4)
				);
			}
			else
			{
				LAPP.SetResolution(
					(fuInt)luaL_checkinteger(L, 1),
					(fuInt)luaL_checkinteger(L, 2)
				);
			}
			return 0;
		}
		static int SetPreferenceGPU(lua_State* L)LNOEXCEPT
		{
			LAPP.SetPreferenceGPU(luaL_checkstring(L, 1), lua_toboolean(L, 2));
			return 0;
		}
		static int SetFPS(lua_State* L)LNOEXCEPT
		{
			int v = luaL_checkinteger(L, 1);
			if (v <= 0)
				v = 60;
			LAPP.SetFPS(static_cast<fuInt>(v));
			return 0;
		}
		static int GetFPS(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, LAPP.GetFPS());
			return 1;
		}
		static int Log(lua_State* L)LNOEXCEPT
		{
			const lua_Integer level = luaL_checkinteger(L, 1);
			const char* msg = luaL_checkstring(L, 2);
			spdlog::log((spdlog::level::level_enum)level, "[lua] {}", msg);
			return 0;
		}
		static int SystemLog(lua_State* L)LNOEXCEPT
		{
			const char* msg = luaL_checkstring(L, 1);
			spdlog::info("[lua] {}", msg);
			return 0;
		}
		static int Print(lua_State* L)LNOEXCEPT
		{
			int n = lua_gettop(L);
			lua_getglobal(L, "tostring"); // ... f
			lua_pushstring(L, ""); // ... f s
			for (int i = 1; i <= n; i++)
			{
				if (i > 1)
				{
					lua_pushstring(L, "\t"); // ... f s s
					lua_concat(L, 2); // ... f s
				}
				lua_pushvalue(L, -2); // ... f s f
				lua_pushvalue(L, i); // ... f s f arg[i]
				lua_call(L, 1, 1); // ... f s ret
				const char* x = luaL_checkstring(L, -1);
				lua_concat(L, 2); // ... f s
			}
			const char* msg = luaL_checkstring(L, -1);
			spdlog::info("[lua] {}", msg);
			return 0;
		}
		static int DoFile(lua_State* L)LNOEXCEPT
		{
			int args = lua_gettop(L);//获取此时栈上的值的数量
			LAPP.LoadScript(luaL_checkstring(L, 1),luaL_optstring(L,2,NULL));
			return (lua_gettop(L)- args);
		}
		static int LoadTextFile(lua_State* L)LNOEXCEPT
		{
			return LAPP.LoadTextFile(L, luaL_checkstring(L, 1), luaL_optstring(L, 2, NULL));
		}
		#pragma endregion
		
		#pragma region 窗口与交换链控制函数
		// 窗口与交换链控制函数
		static int ChangeVideoMode(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) <= 4)
			{
				lua_pushboolean(L, LAPP.ChangeVideoMode(
					luaL_checkinteger(L, 1),
					luaL_checkinteger(L, 2),
					lua_toboolean(L, 3) == 0 ? false : true,
					lua_toboolean(L, 4) == 0 ? false : true
				));
			}
			else
			{
				lua_pushboolean(L, LAPP.ChangeVideoMode2(
					luaL_checkinteger(L, 1),
					luaL_checkinteger(L, 2),
					lua_toboolean(L, 3) == 0 ? false : true,
					lua_toboolean(L, 4) == 0 ? false : true,
					luaL_checkinteger(L, 5),
					luaL_optinteger(L, 6, 1),
					lua_toboolean(L, 7)
				));
			}
			return 1;
		}
		static int SetSwapChainSize(lua_State* L)LNOEXCEPT
		{
			const fuInt width = (fuInt)luaL_checkinteger(L, 1);
			const fuInt height = (fuInt)luaL_checkinteger(L, 2);
			const fBool windowed = lua_toboolean(L, 3);
			const fBool vsync = lua_toboolean(L, 4);
			const fBool flip = lua_toboolean(L, 5);
			fResult result = LAPP.GetRenderDev()->SetBufferSize(width, height, windowed, vsync, flip, F2DAALEVEL_NONE);
			if (result == FCYERR_OK)
			{
				spdlog::info("[fancy2d] 交换链更新成功 Size:({}x{}) Windowed:{} Vsync:{} Flip:{}",
					width, height, windowed, vsync, flip);
			}
			else
			{
				spdlog::error("[fancy2d] [f2dRenderDevice::SetBufferSize] 交换链更新失败(fResult={})，参数为 Size:({}x{}) Windowed:{} Vsync:{} Flip:{}",
					result,
					width, height, windowed, vsync, flip);
			}
			lua_pushboolean(L, result == FCYERR_OK);
			return 1;
		}
		static int SetDisplayMode(lua_State* L)LNOEXCEPT
		{
			const fuInt width = (fuInt)luaL_checkinteger(L, 1);
			const fuInt height = (fuInt)luaL_checkinteger(L, 2);
			const fuInt refreshrate = (fuInt)luaL_checkinteger(L, 3);
			const fBool windowed = lua_toboolean(L, 4);
			const fBool vsync = lua_toboolean(L, 5);
			const fBool flip = lua_toboolean(L, 6);
			const fuInt refreshrateb = (fuInt)luaL_optinteger(L, 7, 1);
			fResult result = LAPP.GetRenderDev()->SetDisplayMode(width, height, refreshrate, refreshrateb, windowed, vsync, flip);
			if (result == FCYERR_OK)
			{
				spdlog::info("[fancy2d] 交换链更新成功 Size:({}x{}) RefreshRate:{} Windowed:{} Vsync:{} Flip:{}",
					width, height, (float)refreshrate / (float)refreshrateb, windowed, vsync, flip);
			}
			else
			{
				spdlog::error("[fancy2d] [f2dRenderDevice::SetDisplayMode] 交换链更新失败(fResult={})，参数为 Size:({}x{}) RefreshRate:{} Windowed:{} Vsync:{} Flip:{}",
					result,
					width, height, (float)refreshrate / (float)refreshrateb, windowed, vsync, flip);
			}
			lua_pushboolean(L, result == FCYERR_OK);
			return 1;
		}
		static int EnumResolutions(lua_State* L) {
			if (LAPP.GetRenderDev())
			{
				auto count = LAPP.GetRenderDev()->GetSupportedDisplayModeCount(true);
				lua_createtable(L, count, 0);		// t
				for (auto index = 0; index < count; index++)
				{
					f2dDisplayMode mode = LAPP.GetRenderDev()->GetSupportedDisplayMode(index);

					lua_createtable(L, 7, 0);		// t t

					lua_pushinteger(L, (lua_Integer)mode.width);
					lua_rawseti(L, -2, 1);

					lua_pushinteger(L, (lua_Integer)mode.height);
					lua_rawseti(L, -2, 2);

					lua_pushnumber(L, (lua_Number)mode.refresh_rate.numerator); // 有点担心存不下
					lua_rawseti(L, -2, 3);

					lua_pushnumber(L, (lua_Number)mode.refresh_rate.denominator); // 有点担心存不下
					lua_rawseti(L, -2, 4);

					lua_pushinteger(L, (lua_Integer)mode.format);
					lua_rawseti(L, -2, 5);

					lua_pushinteger(L, (lua_Integer)mode.scanline_ordering);
					lua_rawseti(L, -2, 6);

					lua_pushinteger(L, (lua_Integer)mode.scaling);
					lua_rawseti(L, -2, 7);

					lua_rawseti(L, -2, index + 1);	// t
				}
				return 1;
			}
			else
			{
				return luaL_error(L, "render device is not avilable.");
			}
		}
		static int EnumGPUs(lua_State* L) {
			if (LAPP.GetRenderDev())
			{
				auto count = LAPP.GetRenderDev()->GetSupportedDeviceCount();
				lua_createtable(L, count, 0);		// t
				for (auto index = 0; index < count; index++)
				{
					lua_pushstring(L, LAPP.GetRenderDev()->GetSupportedDeviceName(index)); // t name
					lua_rawseti(L, -2, index + 1);	// t
				}
				return 1;
			}
			else
			{
				return luaL_error(L, "render device is not avilable.");
			}
		}
		#pragma endregion
		
		#pragma region 内置数学库
		static int Sin(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, sin(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int Cos(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, cos(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int ASin(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, asin(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int ACos(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, acos(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int Tan(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, tan(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int ATan(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, atan(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int ATan2(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, atan2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)) * LRAD2DEGREE);
			return 1;
		}
		#pragma endregion
		
		static int _Empty(lua_State* L) noexcept {
			return 0;
		}
	};
	
	luaL_Reg tFunctions[] = {
		#pragma region 框架函数
		{ "GetVersionNumber", &WrapperImplement::GetVersionNumber },
		{ "GetVersionName", &WrapperImplement::GetVersionName },
		{ "SetWindowed", &WrapperImplement::SetWindowed },
		{ "SetFPS", &WrapperImplement::SetFPS },
		{ "GetFPS", &WrapperImplement::GetFPS },
		{ "SetVsync", &WrapperImplement::SetVsync },
		{ "SetPreferenceGPU", &WrapperImplement::SetPreferenceGPU },
		{ "SetResolution", &WrapperImplement::SetResolution },
		{ "ShowSplashWindow", &WrapperImplement::_Empty },
		{ "Log", &WrapperImplement::Log },
		{ "SystemLog", &WrapperImplement::SystemLog },
		{ "Print", &WrapperImplement::Print },
		{ "DoFile", &WrapperImplement::DoFile },
		{ "LoadTextFile", &WrapperImplement::LoadTextFile },
		#pragma endregion
		
		#pragma region 窗口与交换链控制函数
		{ "ChangeVideoMode", &WrapperImplement::ChangeVideoMode },
		{ "SetSwapChainSize", &WrapperImplement::SetSwapChainSize },
		{ "SetDisplayMode", &WrapperImplement::SetDisplayMode },
		{ "EnumResolutions", &WrapperImplement::EnumResolutions },
		{ "EnumGPUs", &WrapperImplement::EnumGPUs },
		#pragma endregion
		
		#pragma region 内置数学函数
		{ "sin", &WrapperImplement::Sin },
		{ "cos", &WrapperImplement::Cos },
		{ "asin", &WrapperImplement::ASin },
		{ "acos", &WrapperImplement::ACos },
		{ "tan", &WrapperImplement::Tan },
		{ "atan", &WrapperImplement::ATan },
		{ "atan2", &WrapperImplement::ATan2 },
		#pragma endregion
		
		{ NULL, NULL },
	};
	
	::luaL_register(L, LUASTG_LUA_LIBNAME, tFunctions);	// ? t
	lua_pop(L, 1);										// ?
}
