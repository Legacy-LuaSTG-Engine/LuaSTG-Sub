#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/PostEffectShader.hpp"
#include "LuaBinding/Resource.hpp"
#include "LuaBinding/external/HttpClient.hpp"
#include "LuaBinding/modern/Clipboard.hpp"
#include "LuaBinding/modern/Display.hpp"
#include "LuaBinding/modern/Window.hpp"
#include "LuaBinding/modern/SwapChain.hpp"
#include "LuaBinding/modern/Texture2D.hpp"
#include "LuaBinding/modern/Mesh.hpp"
#include "LuaBinding/modern/MeshRenderer.hpp"
#include "LuaBinding/modern/Vector2.hpp"
#include "LuaBinding/modern/Vector3.hpp"
#include "LuaBinding/modern/Vector4.hpp"
#include "LuaBinding/modern/Sprite.hpp"
#include "LuaBinding/modern/SpriteRenderer.hpp"
#include "LuaBinding/modern/FileSystemWatcher.hpp"
#include "LuaBinding/modern/GameObject.hpp"

namespace luastg::binding
{
	static int lib_StopWatch(lua_State* L) noexcept
	{
		StopWatch::CreateAndPush(L);
		return 1;
	}
	static int lib_Rand(lua_State* L) noexcept
	{
		Randomizer::CreateAndPush(L);
		return 1;
	}
	static int lib_BentLaser(lua_State* L) noexcept
	{
		BentLaser::CreateAndPush(L);
		return 1;
	}

	void RegistBuiltInClassWrapper(lua_State* L) noexcept
	{
		luaL_Reg constructors[] = {
			{ "StopWatch", &lib_StopWatch },
			{ "Rand", &lib_Rand },
			{ "BentLaserData", &lib_BentLaser },
			{ nullptr, nullptr },
		};

		luaL_register(L, LUASTG_LUA_LIBNAME, constructors);	// ? t
		Color::Register(L);
		ParticleSystem::Register(L);
		StopWatch::Register(L);
		Randomizer::Register(L);
		BentLaser::Register(L);
		DirectInput::Register(L);
		lua_pop(L, 1);	

		BuiltInFunction::Register(L);  // 内建函数库
		Input::Register(L);
		Render::Register(L);
		Renderer::Register(L);
		GameObjectManager::Register(L);
		ResourceManager::Register(L);
		Audio::Register(L);
		Platform::Register(L);
		FileManager::Register(L); //内建函数库，文件资源管理，请确保位于内建函数库后加载
		Archive::Register(L); //压缩包
		lua_settop(L, 0);
		luaopen_LuaSTG_Sub(L);

		// external

		PostEffectShader::Register(L);
		http::Request::registerClass(L);
		http::ResponseEntity::registerClass(L);

		// modern

		Clipboard::registerClass(L);
		Display::registerClass(L);
		Window::registerClass(L);
		Window_InputMethodExtension::registerClass(L);
		Window_TextInputExtension::registerClass(L);
		Window_Windows11Extension::registerClass(L);
		SwapChain::registerClass(L);
		Texture2D::registerClass(L);
		Mesh::registerClass(L);
		MeshRenderer::registerClass(L);
		Vector2::registerClass(L);
		Vector3::registerClass(L);
		Vector4::registerClass(L);
		Sprite::registerClass(L);
		SpriteRenderer::registerClass(L);
		SpriteRectRenderer::registerClass(L);
		SpriteQuadRenderer::registerClass(L);
		FileSystemWatcher::registerClass(L);
		GameObject::registerClass(L);
	}
}
