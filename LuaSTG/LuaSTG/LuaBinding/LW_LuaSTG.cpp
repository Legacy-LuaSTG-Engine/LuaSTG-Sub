#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "AppFrame.h"

void LuaSTGPlus::BuiltInFunctionWrapper::Register(lua_State* L)noexcept
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
		static int SetWindowed(lua_State* L)noexcept
		{
			LAPP.SetWindowed(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetVsync(lua_State* L)noexcept
		{
			LAPP.SetVsync(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetResolution(lua_State* L)noexcept
		{
			if (lua_gettop(L) >= 4)
			{
				LAPP.SetResolution(
					(uint32_t)luaL_checkinteger(L, 1),
					(uint32_t)luaL_checkinteger(L, 2),
					(uint32_t)luaL_checknumber(L, 3),
					(uint32_t)luaL_checknumber(L, 4)
				);
			}
			else
			{
				LAPP.SetResolution(
					(uint32_t)luaL_checkinteger(L, 1),
					(uint32_t)luaL_checkinteger(L, 2)
				);
			}
			return 0;
		}
		static int SetPreferenceGPU(lua_State* L)noexcept
		{
			LAPP.SetPreferenceGPU(luaL_checkstring(L, 1), lua_toboolean(L, 2));
			return 0;
		}
		static int SetFPS(lua_State* L)noexcept
		{
			int v = luaL_checkinteger(L, 1);
			if (v <= 0)
				v = 60;
			LAPP.SetFPS((uint32_t)v);
			return 0;
		}
		static int GetFPS(lua_State* L)noexcept
		{
			lua_pushnumber(L, LAPP.GetFPS());
			return 1;
		}
		static int Log(lua_State* L)noexcept
		{
			lua_Integer const level = luaL_checkinteger(L, 1);
			std::string_view const msg = luaL_check_string_view(L, 2);
			spdlog::log((spdlog::level::level_enum)level, "[lua] {}", msg);
			return 0;
		}
		static int DoFile(lua_State* L)noexcept
		{
			int args = lua_gettop(L);//获取此时栈上的值的数量
			LAPP.LoadScript(luaL_checkstring(L, 1), luaL_optstring(L, 2, NULL));
			return (lua_gettop(L) - args);
		}
		static int LoadTextFile(lua_State* L)noexcept
		{
			return LAPP.LoadTextFile(L, luaL_checkstring(L, 1), luaL_optstring(L, 2, NULL));
		}
		#pragma endregion
		
		#pragma region 窗口与交换链控制函数
		// 窗口与交换链控制函数
		static int ChangeVideoMode(lua_State* L)noexcept
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
		static int SetDisplayMode(lua_State* L) noexcept
		{
			bool const windowed = lua_toboolean(L, 1);
			int const width = (int)luaL_checkinteger(L, 2);
			int const height = (int)luaL_checkinteger(L, 3);
			bool const vsync = lua_toboolean(L, 4);

			if (width < 1 || height < 1)
				return luaL_error(L, "width and height must lager than 0");

			LAPP.GetAppModel()->getSwapChain()->setVSync(vsync);
			bool result = false;
			if (windowed)
			{
				bool const flip = lua_toboolean(L, 5);
				result = LAPP.GetAppModel()->getSwapChain()->setWindowMode((uint32_t)width, (uint32_t)height, flip);
			}
			else
			{
				int const a = (int)luaL_checkinteger(L, 5);
				int const b = (int)luaL_checkinteger(L, 6);
				Core::Graphics::DisplayMode mode = {
					.width = (uint32_t)width,
					.height = (uint32_t)height,
					.refresh_rate = Core::Rational((uint32_t)a, (uint32_t)b),
					.format = Core::Graphics::Format::B8G8R8A8_UNORM, // 未使用
				};
				if (LAPP.GetAppModel()->getSwapChain()->findBestMatchDisplayMode(mode))
				{
					result = LAPP.GetAppModel()->getSwapChain()->setExclusiveFullscreenMode(mode);
				}
			}
			lua_pushboolean(L, result);
			return 1;
		}
		static int EnumResolutions(lua_State* L) {
			if (LAPP.GetAppModel())
			{
				auto* p_swapchain = LAPP.GetAppModel()->getSwapChain();
				p_swapchain->refreshDisplayMode();
				auto count = p_swapchain->getDisplayModeCount();
				lua_createtable(L, count, 0);		// t
				for (int index = 0; index < (int)count; index += 1)
				{
					auto mode = p_swapchain->getDisplayMode((uint32_t)index);

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

					lua_pushinteger(L, 0); // legacy
					lua_rawseti(L, -2, 6);

					lua_pushinteger(L, 0); // legacy
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
			if (LAPP.GetAppModel())
			{
				auto* p_device = LAPP.GetAppModel()->getDevice();
				auto count = p_device->getGpuCount();
				lua_createtable(L, count, 0);		// t
				for (int index = 0; index < (int)count; index += 1)
				{
					lua_push_string_view(L, p_device->getGpuName((uint32_t)index)); // t name
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
		static int Sin(lua_State* L)noexcept
		{
			lua_pushnumber(L, sin(luaL_checknumber(L, 1) * L_DEG_TO_RAD));
			return 1;
		}
		static int Cos(lua_State* L)noexcept
		{
			lua_pushnumber(L, cos(luaL_checknumber(L, 1) * L_DEG_TO_RAD));
			return 1;
		}
		static int ASin(lua_State* L)noexcept
		{
			lua_pushnumber(L, asin(luaL_checknumber(L, 1)) * L_RAD_TO_DEG);
			return 1;
		}
		static int ACos(lua_State* L)noexcept
		{
			lua_pushnumber(L, acos(luaL_checknumber(L, 1)) * L_RAD_TO_DEG);
			return 1;
		}
		static int Tan(lua_State* L)noexcept
		{
			lua_pushnumber(L, tan(luaL_checknumber(L, 1) * L_DEG_TO_RAD));
			return 1;
		}
		static int ATan(lua_State* L)noexcept
		{
			lua_pushnumber(L, atan(luaL_checknumber(L, 1)) * L_RAD_TO_DEG);
			return 1;
		}
		static int ATan2(lua_State* L)noexcept
		{
			lua_pushnumber(L, atan2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)) * L_RAD_TO_DEG);
			return 1;
		}
		#pragma endregion
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
		{ "Log", &WrapperImplement::Log },
		{ "DoFile", &WrapperImplement::DoFile },
		{ "LoadTextFile", &WrapperImplement::LoadTextFile },
		#pragma endregion
		
		#pragma region 窗口与交换链控制函数
		{ "ChangeVideoMode", &WrapperImplement::ChangeVideoMode },
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
	
	luaL_register(L, LUASTG_LUA_LIBNAME, tFunctions);	// ? t
	lua_pop(L, 1);										// ?
}
