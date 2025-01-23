#include "LuaBinding/LuaWrapper.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"

inline Core::RectI lua_to_Core_RectI(lua_State* L, int idx)
{
	if (!lua_istable(L, idx))
	{
		return Core::RectI();
	}

	Core::Vector2I pos;

	lua_getfield(L, idx, "x");
	pos.x = (int32_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, idx, "y");
	pos.y = (int32_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	Core::Vector2I size;

	lua_getfield(L, idx, "width");
	size.x = (int32_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, idx, "height");
	size.y = (int32_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	return Core::RectI(pos, pos + size);
}

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
			LAPP.SetWindowed(lua_toboolean(L, 1));
			return 0;
		}
		static int SetVsync(lua_State* L)noexcept
		{
			LAPP.SetVsync(lua_toboolean(L, 1));
			return 0;
		}
		static int SetResolution(lua_State* L)noexcept
		{
			LAPP.SetResolution(
				(uint32_t)luaL_checkinteger(L, 1),
				(uint32_t)luaL_checkinteger(L, 2)
			);
			return 0;
		}
		static int SetPreferenceGPU(lua_State* L)noexcept
		{
			LAPP.SetPreferenceGPU(luaL_checkstring(L, 1));
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
			lua::stack_t S(L);
			auto const level = S.get_value<int32_t>(1);
			auto const message = S.get_value<std::string_view>(2);
			spdlog::log(static_cast<spdlog::level::level_enum>(level), "[lua] {}", message);
			return 0;
		}
		static int DoFile(lua_State* L)noexcept
		{
			int args = lua_gettop(L);//获取此时栈上的值的数量
			LAPP.LoadScript(L, luaL_checkstring(L, 1), luaL_optstring(L, 2, NULL));
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
			uint32_t const width = (uint32_t)luaL_checkinteger(L, 1);
			uint32_t const height = (uint32_t)luaL_checkinteger(L, 2);
			bool const windowed = lua_toboolean(L, 3);
			bool const vsync = lua_toboolean(L, 4);

			auto const size = Core::Vector2U(width, height);

			if (windowed)
			{
				bool const result = LAPP.SetDisplayModeWindow(size, vsync);
				lua_pushboolean(L, result);
			}
			else
			{
				bool const result = LAPP.SetDisplayModeExclusiveFullscreen(size, vsync, Core::Rational());
				lua_pushboolean(L, result);
			}

			return 1;
		}
		static int EnumResolutions(lua_State* L)
		{
			lua_createtable(L, 5, 0);		// t
			Core::Graphics::DisplayMode mode_list[5] = {
				{ 640, 480, { 60, 1 }, Core::Graphics::Format::B8G8R8A8_UNORM },
				{ 800, 600, { 60, 1 }, Core::Graphics::Format::B8G8R8A8_UNORM },
				{ 960, 720, { 60, 1 }, Core::Graphics::Format::B8G8R8A8_UNORM },
				{ 1024, 768, { 60, 1 }, Core::Graphics::Format::B8G8R8A8_UNORM },
				{ 1280, 960, { 60, 1 }, Core::Graphics::Format::B8G8R8A8_UNORM },
			};
			for (int index = 0; index < 5; index += 1)
			{
				auto mode = mode_list[index];

				lua_createtable(L, 4, 0);		// t t

				lua_pushinteger(L, (lua_Integer)mode.width);
				lua_rawseti(L, -2, 1);

				lua_pushinteger(L, (lua_Integer)mode.height);
				lua_rawseti(L, -2, 2);

				lua_pushnumber(L, (lua_Number)mode.refresh_rate.numerator); // 有点担心存不下
				lua_rawseti(L, -2, 3);

				lua_pushnumber(L, (lua_Number)mode.refresh_rate.denominator); // 有点担心存不下
				lua_rawseti(L, -2, 4);

				lua_rawseti(L, -2, index + 1);	// t
			}
			return 1;
		}
		static int EnumGPUs(lua_State* L) {
			lua::stack_t S(L);
			if (LAPP.GetAppModel())
			{
				auto* p_device = LAPP.GetAppModel()->getDevice();
				auto count = p_device->getGpuCount();
				lua_createtable(L, count, 0);		// t
				for (int index = 0; index < (int)count; index += 1)
				{
					S.push_value(p_device->getGpuName((uint32_t)index)); // t name
					lua_rawseti(L, -2, index + 1);	// t
				}
				return 1;
			}
			else
			{
				return luaL_error(L, "render device is not avilable.");
			}
		}
		static int ChangeGPU(lua_State* L) {
			lua::stack_t S(L);
			if (LAPP.GetAppModel())
			{
				auto const gpu = S.get_value<std::string_view>(1);
				auto* p_device = LAPP.GetAppModel()->getDevice();
				p_device->setPreferenceGpu(gpu);
				if (!p_device->recreate())
					return luaL_error(L, "ChangeGPU failed.");
				return 0;
			}
			else
			{
				return luaL_error(L, "render device is not avilable.");
			}
		}
		static int GetCurrentGpuName(lua_State* L)
		{
			if (!LAPP.GetAppModel()->getDevice())
			{
				return luaL_error(L, "render device is not avilable.");
			}
			lua::stack_t S(L);
			auto const name = LAPP.GetAppModel()->getDevice()->getCurrentGpuName();
			S.push_value<std::string_view>(name);
			return 1;
		}
		static int SetSwapChainScalingMode(lua_State* L)noexcept
		{
			LAPP.GetAppModel()->getSwapChain()->setScalingMode(
				(Core::Graphics::SwapChainScalingMode)luaL_checkinteger(L, 1));
			return 0;
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
		{ "EnumResolutions", &WrapperImplement::EnumResolutions },
		{ "EnumGPUs", &WrapperImplement::EnumGPUs },
		{ "ChangeGPU", &WrapperImplement::ChangeGPU },
		{ "GetCurrentGpuName", &WrapperImplement::GetCurrentGpuName },
		{ "SetSwapChainScalingMode", &WrapperImplement::SetSwapChainScalingMode },
		#pragma endregion
		
		{ NULL, NULL },
	};
	
	luaL_register(L, LUASTG_LUA_LIBNAME, tFunctions);	// ? t
	lua_pop(L, 1);										// ?
}
