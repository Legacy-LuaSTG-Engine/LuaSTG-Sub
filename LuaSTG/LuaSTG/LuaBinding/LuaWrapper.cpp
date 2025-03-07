#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/PostEffectShader.hpp"
#include "LuaBinding/Display.hpp"
#include "LuaBinding/Window.hpp"
#include "LuaBinding/SwapChain.hpp"
#include "LuaBinding/HttpClient.hpp"
#include "LuaBinding/Texture2D.hpp"
#include "LuaBinding/Mesh.hpp"
#include "LuaBinding/MeshRenderer.hpp"
#include "LuaBinding/Vector2.hpp"

namespace LuaSTGPlus
{
	void LuaWrapper::Register(lua_State* L) noexcept
	{
		struct Function
		{
			static int StopWatch(lua_State* L) noexcept
			{
				StopWatchWrapper::CreateAndPush(L);
				return 1;
			}
			static int Rand(lua_State* L) noexcept
			{
				RandomizerWrapper::CreateAndPush(L);
				return 1;
			}
			static int BentLaser(lua_State* L) noexcept
			{
				BentLaserWrapper::CreateAndPush(L);
				return 1;
			}
		};
			
		luaL_Reg tMethod[] =
		{
			{ "StopWatch", &Function::StopWatch },
			{ "Rand", &Function::Rand },
			{ "BentLaserData", &Function::BentLaser },
			{ NULL, NULL }
		};

		luaL_register(L, LUASTG_LUA_LIBNAME, tMethod);	// ? t
		ColorWrapper::Register(L);
		ParticleSystemWrapper::Register(L);
		StopWatchWrapper::Register(L);
		RandomizerWrapper::Register(L);
		BentLaserWrapper::Register(L);
		DInputWrapper::Register(L);
		lua_pop(L, 1);									// ?
	}
	
	void RegistBuiltInClassWrapper(lua_State* L) noexcept
	{
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
		LuaSTG::LuaBinding::PostEffectShader::Register(L);
		LuaSTG::Sub::LuaBinding::Display::registerClass(L);
		LuaSTG::Sub::LuaBinding::Window::registerClass(L);
		LuaSTG::Sub::LuaBinding::Window_InputMethodExtension::registerClass(L);
		LuaSTG::Sub::LuaBinding::Window_TextInputExtension::registerClass(L);
		LuaSTG::Sub::LuaBinding::Window_Windows11Extension::registerClass(L);
		LuaSTG::Sub::LuaBinding::SwapChain::registerClass(L);
		LuaSTG::Sub::LuaBinding::Texture2D::registerClass(L);
		LuaSTG::Sub::LuaBinding::Mesh::registerClass(L);
		LuaSTG::Sub::LuaBinding::MeshRenderer::registerClass(L);
		LuaSTG::Sub::LuaBinding::Vector2::registerClass(L);
		http::Request::registerClass(L);
		http::ResponseEntity::registerClass(L);
	}
}
