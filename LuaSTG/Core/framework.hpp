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
#include <unordered_map>
#include <thread>

#define NOMINMAX
#include <Windows.h>
#include <VersionHelpers.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
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

#include "spdlog/spdlog.h"

#include "Tracy.hpp"
#include "TracyD3D11.hpp"
#include "TracyD3D11Ctx.hpp"

#include "Platform/HResultChecker.hpp"

namespace Microsoft::WRL::Wrappers
{
	namespace HandleTraits
	{
		struct ThreadHandleTraits : HANDLENullTraits
		{
		};
	}
	using ThreadHandle = HandleT<HandleTraits::ThreadHandleTraits>;
}

#ifdef _DEBUG
inline void F_D3D_SET_DEBUG_NAME(IDXGIObject* ptr, std::string_view file, int line, std::string_view name)
{
	if (ptr)
	{
		std::stringstream strstr;
		strstr << "File: " << file << " ";
		strstr << "Line: " << line << " ";
		strstr << "DXGIObject: " << name;
		std::string str = strstr.str();
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
	}
}
inline void F_D3D_SET_DEBUG_NAME(ID3D11Device* ptr, std::string_view file, int line, std::string_view name)
{
	if (ptr)
	{
		std::stringstream strstr;
		strstr << "File: " << file << " ";
		strstr << "Line: " << line << " ";
		strstr << "DXGIObject: " << name;
		std::string str = strstr.str();
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
	}
}
inline void F_D3D_SET_DEBUG_NAME(ID3D11DeviceChild* ptr, std::string_view file, int line, std::string_view name)
{
	if (ptr)
	{
		std::stringstream strstr;
		strstr << "File: " << file << " ";
		strstr << "Line: " << line << " ";
		strstr << "DXGIObject: " << name;
		std::string str = strstr.str();
		ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)str.length(), str.c_str());
	}
}
#define M_D3D_SET_DEBUG_NAME(OBJ, STR) F_D3D_SET_DEBUG_NAME(OBJ, __FILE__, __LINE__, STR)
#define M_D3D_SET_DEBUG_NAME_SIMPLE(OBJ) M_D3D_SET_DEBUG_NAME(OBJ, #OBJ)
#else
#define M_D3D_SET_DEBUG_NAME(OBJ, STR)
#define M_D3D_SET_DEBUG_NAME_SIMPLE(OBJ)
#endif
