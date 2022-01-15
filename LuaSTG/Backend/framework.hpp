#pragma once

#include <algorithm>

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "spdlog/spdlog.h"

namespace LuaSTG
{
	// Helper struct
	struct HResultCheck
	{
		HRESULT hr = 0;
		char const* cfile = nullptr;
		wchar_t const* file = nullptr;
		int line = 0;
		wchar_t const* message = nullptr;
		HRESULT operator=(HRESULT v);
		static HResultCheck& get(char const* cfile, wchar_t const* file, int line, wchar_t const* message);
	};
	struct HResultToBool
	{
		HRESULT hr = 0;
		bool operator=(HRESULT v);
		static HResultToBool& get();
	};
}

#ifdef _DEBUG
#define gHR HResultCheck::get(__FILE__, __FILEW__, __LINE__, L"")
#define cHR(__ERROR_MESSAGE__) HResultCheck::get(__FILE__, __FILEW__, __LINE__, __ERROR_MESSAGE__)
#else
#define gHR HResultCheck::get(nullptr, nullptr, 0, L"")
#define cHR(__ERROR_MESSAGE__) HResultCheck::get(nullptr, nullptr, 0, __ERROR_MESSAGE__)
#endif

#define bHR HResultToBool::get()
