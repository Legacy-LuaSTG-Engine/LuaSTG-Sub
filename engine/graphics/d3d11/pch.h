#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#include <sdkddkver.h>

#include <cassert>
#include <atomic>
#include <string>
#include <string_view>
#include <span>
#include <array>
#include <memory_resource>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <thread>
#include <format>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <ntstatus.h>
#include <win32/base.hpp>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <DirectXMath.h>

#include "tracy/Tracy.hpp"
#include "tracy/TracyD3D11.hpp"
#include "tracy/TracyAPI.hpp"

#include "windows/HResultChecker.hpp"
