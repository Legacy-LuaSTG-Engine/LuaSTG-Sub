#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#include <cassert>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSERVICE
#define NOMCX

#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

namespace Microsoft::WRL::Wrappers
{
	using ThreadHandle = HandleT<HandleTraits::HANDLENullTraits>;
	using SemaphoreHandle = HandleT<HandleTraits::HANDLENullTraits>;
}

// Now, let's FUCK Windows.h

#undef CreateWindow
#undef DefWindowProc
#undef DrawText
#undef MessageBox

#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d2d1_3.h>
#include <dcomp.h>
#include <dwrite_3.h>

#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef NOSERVICE
#undef NOMCX
