#include "CLRBinding.hpp"
#include "LuaWrapper.hpp"
#include "AppFrame.h"

inline Core::Graphics::IRenderer* LR2D() { return LAPP.GetAppModel()->getRenderer(); }

void BeginScene()
{
	LR2D()->beginBatch();
}

void EndScene()
{
	LR2D()->endBatch();
}

void RenderClear(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	Core::Color4B color(r, g, b, a);
	LR2D()->clearRenderTarget(color);
}

void Log(int32_t level, intptr_t str)
{
	spdlog::log(static_cast<spdlog::level::level_enum>(level), "[CSharp] {}", std::string_view((char*)str));
}

bool LuaSTGPlus::InitCLRBinding(const CLRHost* host, CLRFunctions* functions)
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

	CLRInitPayload payload{};

	payload.Log = Log;

	payload.BeginScene = BeginScene;
	payload.EndScene = EndScene;
	payload.RenderClear = RenderClear;

	if (((entry_point_fn)fn)(&payload, functions)) 
	{
		return false;
	}

	return true;
}
