#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <unordered_map>
#include <algorithm>

#include "spdlog/spdlog.h"

// Windows
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <Dbt.h> // DBT_DEVNODES_CHANGED
constexpr UINT const WM_MAIN_THREAD_DELEGATE = WM_USER + 0x0010;
constexpr UINT const WM_SET_IME_ENABLE       = WM_USER + 0x0011;
// Direct3D11
#include <dxgi1_6.h>
#include <d3d11_4.h>
// Direct2D1
#include <d2d1_3.h>
#include <dwrite_3.h>
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

// Trace
#include "Tracy.hpp"
#include "TracyD3D11.hpp"
#include "TracyD3D11Ctx.hpp"

// Helper struct
#include "platform/HResultChecker.hpp"
