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

void RenderClear(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
{
	Core::Color4B color(r, g, b, a);
	LR2D()->clearRenderTarget(color);
}

bool LuaSTGPlus::InitCLRBinding(const CLRHost* host, CLRFunctions* functions)
{
	void* fn = nullptr;
	if (host->load_assembly_and_get_function_pointer(
		L".\\Managed\\net8.0\\LuaSTG.dll",
		L"LuaSTG.LuaSTGAPI, LuaSTG",
		L"StartUp",
		UNMANAGEDCALLERSONLY_METHOD,
		&fn) || !fn)
	{
		return false;
	}

	CLRInitPayload payload{};
	payload.BeginScene = BeginScene;
	payload.EndScene = EndScene;
	payload.RenderClear = RenderClear;
	if (((entry_point_fn)fn)(&payload, functions)) 
	{
		return false;
	}

	return true;
}
