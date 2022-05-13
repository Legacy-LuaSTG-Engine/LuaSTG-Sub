#pragma once

#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include "spdlog/spdlog.h"

// Windows
#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
constexpr UINT const WM_MAIN_THREAD_DELEGATE = WM_USER + 0x0010;
constexpr UINT const WM_SET_IME_ENABLE       = WM_USER + 0x0011;
// Direct3D11
#include <dxgi1_6.h>
#include <d3d11_4.h>
// HLSL
#include <d3dcompiler.h>
// Math
#include <DirectXMath.h>
// Image
#include <wincodec.h>
#include "WICTextureLoader11.h"
#include "ScreenGrab11.h"
// Audio
#include "xaudio2redist.h"
#define SAFE_RELEASE_VOICE(x) if (x) { x->DestroyVoice(); x = NULL; }

#include "Tracy.hpp"
#include "TracyD3D11.hpp"

// Helper struct
namespace fancy2d
{
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
#define gHR fancy2d::HResultCheck::get(__FILE__, __FILEW__, __LINE__, L"")
#define cHR(__ERROR_MESSAGE__) fancy2d::HResultCheck::get(__FILE__, __FILEW__, __LINE__, __ERROR_MESSAGE__)
#else
#define gHR fancy2d::HResultCheck::get(nullptr, nullptr, 0, L"")
#define cHR(__ERROR_MESSAGE__) fancy2d::HResultCheck::get(nullptr, nullptr, 0, __ERROR_MESSAGE__)
#endif

#define bHR fancy2d::HResultToBool::get()
