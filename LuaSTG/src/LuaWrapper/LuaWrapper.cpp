#include "LuaWrapper/LuaWrapper.hpp"

namespace LuaSTGPlus
{
	namespace LuaWrapper
	{
		void Register(lua_State* L)LNOEXCEPT {
			struct Function
			{
				static int Color(lua_State* L)LNOEXCEPT
				{
					if (lua_gettop(L) == 1) {
						ColorWrapper::CreateAndPush(L, fcyColor((fuInt)luaL_checknumber(L, 1)));
					}
					else {
						ColorWrapper::CreateAndPush(L, fcyColor(
							(fInt)luaL_checkinteger(L, 1),
							(fInt)luaL_checkinteger(L, 2),
							(fInt)luaL_checkinteger(L, 3),
							(fInt)luaL_checkinteger(L, 4)
						));
					}
					return 1;
				}
				static int HSVColor(lua_State* L)LNOEXCEPT
				{
					ColorWrapper::HSVColor hsv = {
						(float)luaL_checknumber(L, 2),
						(float)luaL_checknumber(L, 3),
						(float)luaL_checknumber(L, 4),
					};
					fcyColor rgb = ColorWrapper::HSV2RGB(hsv);
					rgb.a = (fByte)(std::clamp(luaL_checknumber(L, 1) / 100.0, 0.0, 1.0) * 255.0);
					ColorWrapper::CreateAndPush(L, rgb);
					return 1;
				}
				static int StopWatch(lua_State* L)LNOEXCEPT
				{
					StopWatchWrapper::CreateAndPush(L);
					return 1;
				}
				static int Rand(lua_State* L)LNOEXCEPT
				{
					RandomizerWrapper::CreateAndPush(L);
					return 1;
				}
				static int BentLaser(lua_State* L)LNOEXCEPT
				{
					BentLaserWrapper::CreateAndPush(L);
					return 1;
				}
			};
			
			luaL_Reg tMethod[] =
			{
				{ "Color", &Function::Color },
				{ "HSVColor", &Function::HSVColor },
				{ "StopWatch", &Function::StopWatch },
				{ "Rand", &Function::Rand },
				{ "BentLaserData", &Function::BentLaser },
				{ NULL, NULL }
			};

			::luaL_register(L, LUASTG_LUA_LIBNAME, tMethod);	// ? t
			WindowWrapper::Register(L);
			ColorWrapper::Register(L);
			ParticleSystemWrapper::Register(L);
			StopWatchWrapper::Register(L);
			RandomizerWrapper::Register(L);
			BentLaserWrapper::Register(L);
			DInputWrapper::Register(L);
			IO::Register(L);
			::lua_pop(L, 1);									// ?
		}
	}
	
	void RegistBuiltInClassWrapper(lua_State* L)LNOEXCEPT {
		LuaWrapper::Register(L);
		BuiltInFunctionWrapper::Register(L);  // 内建函数库
		LuaWrapper::InputWrapper::Register(L);
		LuaWrapper::RenderWrapper::Register(L);
		LuaWrapper::RendererWrapper::Register(L);
		LuaWrapper::GameObjectManagerWrapper::Register(L);
		LuaWrapper::ResourceMgrWrapper::Register(L);
		LuaWrapper::AudioWrapper::Register(L);
		LuaWrapper::PlatformWrapper::Register(L);
		FileManagerWrapper::Register(L); //内建函数库，文件资源管理，请确保位于内建函数库后加载
		ArchiveWrapper::Register(L); //压缩包
	}
}
