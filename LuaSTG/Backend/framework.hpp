#pragma once

#include <algorithm>

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <d3d9.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

namespace LuaSTG
{
	// Helper function
	void debugPrintHRESULT(HRESULT hr, const wchar_t* message) noexcept;

	// Helper struct
	struct HResultCheck
	{
		HRESULT hr = 0;
		wchar_t const* file = nullptr;
		int line = 0;
		wchar_t const* message = nullptr;
		HRESULT operator=(HRESULT v);
		static HResultCheck& get(wchar_t const* file, int line, wchar_t const* message);
	};
	struct HResultToBool
	{
		HRESULT hr = 0;
		bool operator=(HRESULT v);
		static HResultToBool& get();
	};
}

#define gHR HResultCheck::get(__FILEW__, __LINE__, L"")
#define cHR(__ERROR_MESSAGE__) HResultCheck::get(__FILEW__, __LINE__, __ERROR_MESSAGE__)
#define bHR HResultToBool::get()
