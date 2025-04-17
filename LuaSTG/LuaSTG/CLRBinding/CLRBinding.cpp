#include "CLRBinding.hpp"
#include "LuaWrapper.hpp"
#include "AppFrame.h"

inline Core::Graphics::IRenderer* LR2D() { return LAPP.GetAppModel()->getRenderer(); }

namespace LuaSTGPlus
{
	void CLRBinding::Log(int32_t level, intptr_t str)
	{
		spdlog::log(static_cast<spdlog::level::level_enum>(level), "[CSharp] {}", std::string_view((char*)str));
	}

	intptr_t CLRBinding::GameObject_New(uint32_t callbackMask)
	{
		return (intptr_t)(GameObjectPool::GetInstance()->CLR_New(callbackMask));
	}

	int32_t CLRBinding::GameObject_GetID(intptr_t p)
	{
		return ((GameObject*)p)->id;
	}

	void CLRBinding::GameObject_DefaultRenderFunc(intptr_t p)
	{
		((GameObject*)p)->Render();
	}

	void CLRBinding::Del(intptr_t p, bool kill_mode) 
	{
		GameObjectPool::GetInstance()->Del((GameObject*)p, kill_mode);
	}

	int32_t CLRBinding::FirstObject(int64_t group_id)
	{
		return GameObjectPool::GetInstance()->FirstObject(static_cast<int>(group_id));
	}

	int32_t CLRBinding::NextObject(int64_t group_id, int32_t id)
	{
		return GameObjectPool::GetInstance()->NextObject(static_cast<int>(group_id), id);
	}

	void CLRBinding::BeginScene()
	{
		LR2D()->beginBatch();
	}

	void CLRBinding::EndScene()
	{
		LR2D()->endBatch();
	}

	void CLRBinding::RenderClear(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
	{
		Core::Color4B color(r, g, b, a);
		LR2D()->clearRenderTarget(color);
	}

	bool InitCLRBinding(const CLRHost* host, ManagedAPI* functions)
	{
		void* fn = nullptr;
		if (host->load_assembly_and_get_function_pointer(
			L".\\Managed\\net8.0\\LuaSTG.dll",
			L"LuaSTG.Core.LuaSTGAPI, LuaSTG.Core",
			L"StartUp",
			UNMANAGEDCALLERSONLY_METHOD,
			&fn) || !fn)
		{
			return false;
		}

		UnmanagedAPI payload{};

		if (((entry_point_fn)fn)(&payload, functions))
		{
			return false;
		}

		return true;
	}
}
