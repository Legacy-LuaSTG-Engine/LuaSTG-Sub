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
#include <vector>
#include <unordered_map>
#include <thread>

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
#include <DirectXMath.h>

#include "spdlog/spdlog.h"

#include "Tracy.hpp"
#include "TracyD3D11.hpp"
#include "TracyD3D11Ctx.hpp"

#include "platform/HResultChecker.hpp"

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
