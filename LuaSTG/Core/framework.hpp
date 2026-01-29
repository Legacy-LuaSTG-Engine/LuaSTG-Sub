#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#include <sdkddkver.h>

#include <string>
#include <string_view>
#include <atomic>
#include <algorithm>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <format>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <ntstatus.h>
#include <Unknwn.h>
#include <win32/base.hpp>
#include <timeapi.h>
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

#ifdef NDEBUG
#define M_D3D_SET_DEBUG_NAME(OBJ, STR)
#define M_D3D_SET_DEBUG_NAME_SIMPLE(OBJ)
#else
inline void F_D3D_SET_DEBUG_NAME(IDXGIObject* ptr, std::string_view file, int line, std::string_view name) {
	if (ptr) {
		const auto str = std::format("{}:{}: IDXGIObject: {}", file, line, name);
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(str.length()), str.c_str());
	}
}
inline void F_D3D_SET_DEBUG_NAME(ID3D11Device* ptr, std::string_view file, int line, std::string_view name) {
	if (ptr) {
		const auto str = std::format("{}:{}: ID3D11Device: {}", file, line, name);
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(str.length()), str.c_str());
	}
}
inline void F_D3D_SET_DEBUG_NAME(ID3D11DeviceChild* ptr, std::string_view file, int line, std::string_view name) {
	if (ptr) {
		const auto str = std::format("{}:{}: ID3D11DeviceChild: {}", file, line, name);
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(str.length()), str.c_str());
	}
}
#define M_D3D_SET_DEBUG_NAME(OBJ, STR) F_D3D_SET_DEBUG_NAME(OBJ, __FILE__, __LINE__, STR)
#define M_D3D_SET_DEBUG_NAME_SIMPLE(OBJ) M_D3D_SET_DEBUG_NAME(OBJ, #OBJ)
#endif
