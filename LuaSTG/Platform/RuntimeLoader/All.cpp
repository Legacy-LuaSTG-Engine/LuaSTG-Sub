#include "Platform/Shared.hpp"
#include "Platform/WindowsVersion.hpp"

#include "Platform/RuntimeLoader/DXGI.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT DXGI::CreateFactory(REFIID riid, void** ppFactory)
	{
		if (api_CreateDXGIFactory2)
		{
			UINT flags = 0;
		#ifdef _DEBUG
			flags |= DXGI_CREATE_FACTORY_DEBUG;
		#endif
			return api_CreateDXGIFactory2(flags, riid, ppFactory);
		}
		else if (api_CreateDXGIFactory1)
		{
			return api_CreateDXGIFactory1(riid, ppFactory);
		}
		else if (api_CreateDXGIFactory)
		{
			return api_CreateDXGIFactory(riid, ppFactory);
		}
		else
		{
			return E_NOTIMPL;
		}
	}

	DXGI::DXGI()
	{
		dll_dxgi = LoadLibraryW(L"dxgi.dll");
		if (dll_dxgi)
		{
			api_CreateDXGIFactory = (decltype(api_CreateDXGIFactory))
				GetProcAddress(dll_dxgi, "CreateDXGIFactory");
			api_CreateDXGIFactory1 = (decltype(api_CreateDXGIFactory1))
				GetProcAddress(dll_dxgi, "CreateDXGIFactory1");
			api_CreateDXGIFactory2 = (decltype(api_CreateDXGIFactory2))
				GetProcAddress(dll_dxgi, "CreateDXGIFactory2");
		}
	}
	DXGI::~DXGI()
	{
		if (dll_dxgi)
		{
			FreeLibrary(dll_dxgi);
		}
		dll_dxgi = NULL;
		api_CreateDXGIFactory = NULL;
		api_CreateDXGIFactory1 = NULL;
		api_CreateDXGIFactory2 = NULL;
	}
}

#include "Platform/RuntimeLoader/Direct3D11.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT Direct3D11::CreateDevice(
		D3D_DRIVER_TYPE DriverType,
		UINT Flags,
		D3D_FEATURE_LEVEL TargetFeatureLevel,
		ID3D11Device** ppDevice,
		D3D_FEATURE_LEVEL* pFeatureLevel,
		ID3D11DeviceContext** ppImmediateContext)
	{
		if (api_D3D11CreateDevice)
		{
		#ifdef _DEBUG
			Flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif
			HRESULT hr = S_OK;
			D3D_FEATURE_LEVEL const d3d_feature_level_list[9] = {
				//D3D_FEATURE_LEVEL_12_2, // no longer supported
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_1,
			};
			UINT const d3d_feature_level_size = 9;
			D3D_FEATURE_LEVEL d3d_feature_level = D3D_FEATURE_LEVEL_9_1;
			for (UINT offset = 0; offset < d3d_feature_level_size; offset += 1)
			{
				hr = api_D3D11CreateDevice(
					NULL,
					DriverType,
					NULL,
					Flags,
					d3d_feature_level_list + offset,
					d3d_feature_level_size - offset,
					D3D11_SDK_VERSION,
					NULL,
					&d3d_feature_level,
					NULL);
				if (SUCCEEDED(hr))
				{
					if ((UINT)d3d_feature_level >= (UINT)TargetFeatureLevel)
					{
						return api_D3D11CreateDevice(
							NULL,
							DriverType,
							NULL,
							Flags,
							d3d_feature_level_list + offset,
							d3d_feature_level_size - offset,
							D3D11_SDK_VERSION,
							ppDevice,
							pFeatureLevel,
							ppImmediateContext);
					}
					else
					{
						return E_NOTIMPL;
					}
				}
			}
			return hr;
		}
		else
		{
			return E_NOTIMPL;
		}
	}
	HRESULT Direct3D11::CreateDevice(
		IDXGIAdapter* pAdapter,
		UINT Flags,
		D3D_FEATURE_LEVEL TargetFeatureLevel,
		ID3D11Device** ppDevice,
		D3D_FEATURE_LEVEL* pFeatureLevel,
		ID3D11DeviceContext** ppImmediateContext)
	{
		if (api_D3D11CreateDevice)
		{
		#ifdef _DEBUG
			Flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif
			HRESULT hr = S_OK;
			D3D_FEATURE_LEVEL const d3d_feature_level_list[9] = {
				//D3D_FEATURE_LEVEL_12_2, // no longer supported
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_1,
			};
			UINT const d3d_feature_level_size = 9;
			D3D_FEATURE_LEVEL d3d_feature_level = D3D_FEATURE_LEVEL_9_1;
			for (UINT offset = 0; offset < d3d_feature_level_size; offset += 1)
			{
				hr = api_D3D11CreateDevice(
					pAdapter,
					D3D_DRIVER_TYPE_UNKNOWN,
					NULL,
					Flags,
					d3d_feature_level_list + offset,
					d3d_feature_level_size - offset,
					D3D11_SDK_VERSION,
					NULL,
					&d3d_feature_level,
					NULL);
				if (SUCCEEDED(hr))
				{
					if ((UINT)d3d_feature_level >= (UINT)TargetFeatureLevel)
					{
						return api_D3D11CreateDevice(
							pAdapter,
							D3D_DRIVER_TYPE_UNKNOWN,
							NULL,
							Flags,
							d3d_feature_level_list + offset,
							d3d_feature_level_size - offset,
							D3D11_SDK_VERSION,
							ppDevice,
							pFeatureLevel,
							ppImmediateContext);
					}
					else
					{
						return E_NOTIMPL;
					}
				}
			}
			return hr;
		}
		else
		{
			return E_NOTIMPL;
		}
	}

	Direct3D11::Direct3D11()
	{
		dll_d3d11 = LoadLibraryW(L"d3d11.dll");
		if (dll_d3d11)
		{
			api_D3D11CreateDevice = (decltype(api_D3D11CreateDevice))
				GetProcAddress(dll_d3d11, "D3D11CreateDevice");
		}
	}
	Direct3D11::~Direct3D11()
	{
		if (dll_d3d11)
		{
			FreeLibrary(dll_d3d11);
		}
		dll_d3d11 = NULL;
		api_D3D11CreateDevice = NULL;
	}
}

#include "Platform/RuntimeLoader/Direct3DCompiler.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT Direct3DCompiler::Compile(LPCVOID pSrcData,
		SIZE_T SrcDataSize,
		LPCSTR pSourceName,
		CONST D3D_SHADER_MACRO* pDefines,
		ID3DInclude* pInclude,
		LPCSTR pEntrypoint,
		LPCSTR pTarget,
		UINT Flags1,
		UINT Flags2,
		ID3DBlob** ppCode,
		ID3DBlob** ppErrorMsgs)
	{
		if (api_D3DCompile)
		{
			return api_D3DCompile(
				pSrcData,
				SrcDataSize,
				pSourceName,
				pDefines,
				pInclude,
				pEntrypoint,
				pTarget,
				Flags1,
				Flags2,
				ppCode,
				ppErrorMsgs);
		}
		return E_NOTIMPL;
	}
	HRESULT Direct3DCompiler::Reflect(LPCVOID pSrcData,
		SIZE_T SrcDataSize,
		REFIID pInterface,
		void** ppReflector)
	{
		if (api_D3DReflect)
		{
			return api_D3DReflect(pSrcData,
				SrcDataSize,
				pInterface,
				ppReflector);
		}
		return E_NOTIMPL;
	}

	Direct3DCompiler::Direct3DCompiler()
	{
		dll_d3dcompiler = LoadLibraryW(L"d3dcompiler_47.dll");
		assert(dll_d3dcompiler);
		if (dll_d3dcompiler)
		{
			api_D3DCompile = (decltype(api_D3DCompile))
				GetProcAddress(dll_d3dcompiler, "D3DCompile");
			api_D3DReflect = (decltype(api_D3DReflect))
				GetProcAddress(dll_d3dcompiler, "D3DReflect");
			assert(api_D3DCompile);
			assert(api_D3DReflect);
		}
	}
	Direct3DCompiler::~Direct3DCompiler()
	{
		if (dll_d3dcompiler)
		{
			FreeLibrary(dll_d3dcompiler);
		}
		dll_d3dcompiler = NULL;
		api_D3DCompile = NULL;
		api_D3DReflect = NULL;
	}
}

#include "Platform/RuntimeLoader/DirectComposition.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT DirectComposition::CreateDevice(IUnknown* renderingDevice, REFIID iid, void** dcompositionDevice)
	{
		if (api_DCompositionCreateDevice3)
		{
			return api_DCompositionCreateDevice3(renderingDevice, iid, dcompositionDevice);
		}
		else if (api_DCompositionCreateDevice2)
		{
			return api_DCompositionCreateDevice2(renderingDevice, iid, dcompositionDevice);
		}
		else if (api_DCompositionCreateDevice)
		{
			HRESULT hr = S_OK;
			IDXGIDevice* dxgiDevice = NULL;
			hr = renderingDevice->QueryInterface(&dxgiDevice);
			if (FAILED(hr)) return hr;
			hr = api_DCompositionCreateDevice(dxgiDevice, iid, dcompositionDevice);
			dxgiDevice->Release();
			return hr;
		}
		else
		{
			return E_NOTIMPL;
		}
	}

	DirectComposition::DirectComposition()
	{
		dll_dcomp = LoadLibraryExW(L"dcomp.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (dll_dcomp)
		{
			api_DCompositionCreateDevice = (decltype(api_DCompositionCreateDevice))
				GetProcAddress(dll_dcomp, "DCompositionCreateDevice");
			api_DCompositionCreateDevice2 = (decltype(api_DCompositionCreateDevice2))
				GetProcAddress(dll_dcomp, "DCompositionCreateDevice2");
			api_DCompositionCreateDevice3 = (decltype(api_DCompositionCreateDevice3))
				GetProcAddress(dll_dcomp, "DCompositionCreateDevice3");
		}
	}
	DirectComposition::~DirectComposition()
	{
		if (dll_dcomp)
		{
			FreeLibrary(dll_dcomp);
		}
		dll_dcomp = NULL;
		api_DCompositionCreateDevice = NULL;
		api_DCompositionCreateDevice2 = NULL;
		api_DCompositionCreateDevice3 = NULL;
	}
}

#include "Platform/RuntimeLoader/Direct2D1.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT Direct2D1::CreateFactory(
		D2D1_FACTORY_TYPE factoryType,
		REFIID riid,
		void** ppIFactory)
	{
		if (api_D2D1CreateFactory)
		{
			D2D1_FACTORY_OPTIONS options = {
			#ifdef _DEBUG
				.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION
			#else
				.debugLevel = D2D1_DEBUG_LEVEL_NONE
			#endif
			};
			return api_D2D1CreateFactory(
				factoryType,
				riid,
				&options,
				ppIFactory);
		}
		return E_NOTIMPL;
	}
	HRESULT Direct2D1::CreateDevice(
		IDXGIDevice* dxgiDevice,
		BOOL multiThread,
		ID2D1Device** d2dDevice)
	{
		if (api_D2D1CreateDevice)
		{
			D2D1_CREATION_PROPERTIES prop = {
				.threadingMode = multiThread
					? D2D1_THREADING_MODE_MULTI_THREADED
					: D2D1_THREADING_MODE_SINGLE_THREADED,
			#ifdef _DEBUG
				.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION,
			#else
				.debugLevel = D2D1_DEBUG_LEVEL_NONE,
			#endif
				.options = multiThread
					? D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS
					: D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			};
			return api_D2D1CreateDevice(dxgiDevice, &prop, d2dDevice);
		}
		return E_NOTIMPL;
	}
	HRESULT Direct2D1::CreateDevice(
		IDXGIDevice* dxgiDevice,
		BOOL multiThread,
		ID2D1Device** d2dDevice,
		ID2D1DeviceContext** d2dDeviceContext)
	{
		if (api_D2D1CreateDevice)
		{
			D2D1_CREATION_PROPERTIES prop = {
				.threadingMode = multiThread
					? D2D1_THREADING_MODE_MULTI_THREADED
					: D2D1_THREADING_MODE_SINGLE_THREADED,
			#ifdef _DEBUG
				.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION,
			#else
				.debugLevel = D2D1_DEBUG_LEVEL_NONE,
			#endif
				.options = multiThread
					? D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS
					: D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			};
			HRESULT hr = api_D2D1CreateDevice(dxgiDevice, &prop, d2dDevice);
			if (FAILED(hr)) return hr;
			return (*d2dDevice)->CreateDeviceContext(
				multiThread
				? D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS
				: D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
				d2dDeviceContext);
		}
		return E_NOTIMPL;
	}

	Direct2D1::Direct2D1()
	{
		dll_d2d1 = LoadLibraryW(L"d2d1.dll");
		if (dll_d2d1)
		{
			api_D2D1CreateFactory = (decltype(api_D2D1CreateFactory))
				GetProcAddress(dll_d2d1, "D2D1CreateFactory");
			api_D2D1CreateDevice = (decltype(api_D2D1CreateDevice))
				GetProcAddress(dll_d2d1, "D2D1CreateDevice");
		}
	}
	Direct2D1::~Direct2D1()
	{
		if (dll_d2d1)
		{
			FreeLibrary(dll_d2d1);
		}
		dll_d2d1 = NULL;
		api_D2D1CreateFactory = NULL;
		api_D2D1CreateDevice = NULL;
	}
}

#include "Platform/RuntimeLoader/DirectWrite.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT DirectWrite::CreateFactory(
		DWRITE_FACTORY_TYPE factoryType,
		REFIID iid,
		void** factory)
	{
		if (api_DWriteCreateFactory)
		{
			return api_DWriteCreateFactory(
				factoryType,
				iid,
				(IUnknown**)factory);
		}
		return E_NOTIMPL;
	}

	DirectWrite::DirectWrite()
	{
		dll_dwrite = LoadLibraryW(L"dwrite.dll");
		if (dll_dwrite)
		{
			api_DWriteCreateFactory = (decltype(api_DWriteCreateFactory))
				GetProcAddress(dll_dwrite, "DWriteCreateFactory");
		}
	}
	DirectWrite::~DirectWrite()
	{
		if (dll_dwrite)
		{
			FreeLibrary(dll_dwrite);
		}
		dll_dwrite = NULL;
		api_DWriteCreateFactory = NULL;
	}
}

#include "Platform/RuntimeLoader/DesktopWindowManager.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT DesktopWindowManager::IsCompositionEnabled(BOOL* pfEnabled)
	{
		if (api_DwmIsCompositionEnabled)
		{
			return api_DwmIsCompositionEnabled(pfEnabled);
		}
		return E_NOTIMPL;
	}
	HRESULT DesktopWindowManager::EnableBlurBehindWindow(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind)
	{
		if (api_DwmEnableBlurBehindWindow)
		{
			return api_DwmEnableBlurBehindWindow(hWnd, pBlurBehind);
		}
		return E_NOTIMPL;
	}
	HRESULT DesktopWindowManager::ExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset)
	{
		if (api_DwmExtendFrameIntoClientArea)
		{
			return api_DwmExtendFrameIntoClientArea(hWnd, pMarInset);
		}
		return E_NOTIMPL;
	}
	HRESULT DesktopWindowManager::GetColorizationColor(DWORD* pcrColorization, BOOL* pfOpaqueBlend)
	{
		if (api_DwmGetColorizationColor)
		{
			return api_DwmGetColorizationColor(pcrColorization, pfOpaqueBlend);
		}
		return E_NOTIMPL;
	}
	BOOL DesktopWindowManager::DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
	{
		if (api_DwmDefWindowProc)
		{
			return api_DwmDefWindowProc(hWnd, msg, wParam, lParam, plResult);
		}
		return FALSE;
	}
	HRESULT DesktopWindowManager::SetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
	{
		if (api_DwmSetWindowAttribute)
		{
			return api_DwmSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
		}
		return E_NOTIMPL;
	}
	HRESULT DesktopWindowManager::Flush()
	{
		if (api_DwmFlush)
		{
			return api_DwmFlush();
		}
		return E_NOTIMPL;
	}

	BOOL DesktopWindowManager::IsWindowTransparencySupported()
	{
		// https://github.com/glfw/glfw/blob/master/src/win32_window.c

		if (!IsWindowsVistaOrGreater())
			return FALSE;

		BOOL composition = FALSE;
		if (FAILED(IsCompositionEnabled(&composition)) || !composition)
			return FALSE;

		if (!IsWindows8OrGreater())
		{
			// HACK: Disable framebuffer transparency on Windows 7 when the
			//       colorization color is opaque, because otherwise the window
			//       contents is blended additively with the previous frame instead
			//       of replacing it
			DWORD color = 0x00000000;
			BOOL opaque = FALSE;
			if (FAILED(GetColorizationColor(&color, &opaque)) || opaque)
				return FALSE;
		}

		return TRUE;
	}
	HRESULT DesktopWindowManager::SetWindowTransparency(HWND hWnd, BOOL bEnable)
	{
		if (bEnable && IsWindowTransparencySupported() && api_CreateRectRgn && api_DeleteObject)
		{
			HRGN region = api_CreateRectRgn(0, 0, -1, -1);
			DWM_BLURBEHIND bb = {};
			bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
			bb.fEnable = TRUE;
			bb.hRgnBlur = region;
			HRESULT const hr = EnableBlurBehindWindow(hWnd, &bb);
			api_DeleteObject(region);
			return hr;
		}
		else
		{
			DWM_BLURBEHIND bb = {};
			bb.dwFlags = DWM_BB_ENABLE;
			return EnableBlurBehindWindow(hWnd, &bb);
		}
	}

	DesktopWindowManager::DesktopWindowManager()
	{
		dll_dwmapi = LoadLibraryW(L"dwmapi.dll");
		dll_gdi32 = LoadLibraryW(L"gdi32.dll");
		assert(dll_dwmapi);
		assert(dll_gdi32);
		if (dll_dwmapi)
		{
			api_DwmIsCompositionEnabled = (decltype(api_DwmIsCompositionEnabled))
				GetProcAddress(dll_dwmapi, "DwmIsCompositionEnabled");
			api_DwmEnableBlurBehindWindow = (decltype(api_DwmEnableBlurBehindWindow))
				GetProcAddress(dll_dwmapi, "DwmEnableBlurBehindWindow");
			api_DwmExtendFrameIntoClientArea = (decltype(api_DwmExtendFrameIntoClientArea))
				GetProcAddress(dll_dwmapi, "DwmExtendFrameIntoClientArea");
			api_DwmGetColorizationColor = (decltype(api_DwmGetColorizationColor))
				GetProcAddress(dll_dwmapi, "DwmGetColorizationColor");
			api_DwmDefWindowProc = (decltype(api_DwmDefWindowProc))
				GetProcAddress(dll_dwmapi, "DwmDefWindowProc");
			api_DwmSetWindowAttribute = (decltype(api_DwmSetWindowAttribute))
				GetProcAddress(dll_dwmapi, "DwmSetWindowAttribute");
			api_DwmFlush = (decltype(api_DwmFlush))
				GetProcAddress(dll_dwmapi, "DwmFlush");
			assert(api_DwmIsCompositionEnabled);
			assert(api_DwmEnableBlurBehindWindow);
			assert(api_DwmExtendFrameIntoClientArea);
			assert(api_DwmGetColorizationColor);
			assert(api_DwmDefWindowProc);
			assert(api_DwmSetWindowAttribute);
			assert(api_DwmFlush);
		}
		if (dll_gdi32)
		{
			api_CreateRectRgn = (decltype(api_CreateRectRgn))
				GetProcAddress(dll_gdi32, "CreateRectRgn");
			api_DeleteObject = (decltype(api_DeleteObject))
				GetProcAddress(dll_gdi32, "DeleteObject");
			assert(api_CreateRectRgn);
			assert(api_DeleteObject);
		}
	}
	DesktopWindowManager::~DesktopWindowManager()
	{
		if (dll_dwmapi)
		{
			FreeLibrary(dll_dwmapi);
		}
		if (dll_gdi32)
		{
			FreeLibrary(dll_gdi32);
		}
		dll_dwmapi = NULL;
		dll_gdi32 = NULL;
		api_DwmIsCompositionEnabled = NULL;
		api_DwmEnableBlurBehindWindow = NULL;
		api_DwmExtendFrameIntoClientArea = NULL;
		api_DwmGetColorizationColor = NULL;
		api_DwmDefWindowProc = NULL;
		api_DwmSetWindowAttribute = NULL;
		api_DwmFlush = NULL;
		api_CreateRectRgn = NULL;
		api_DeleteObject = NULL;
	}
}

#include "Platform/RuntimeLoader/XAudio2.hpp"

namespace Platform::RuntimeLoader
{
	HRESULT XAudio2::Create(IXAudio2** ppXAudio2)
	{
		// 从 Windows 10 1809 开始出现的魔幻 API

		if (api_XAudio2CreateWithVersionInfo)
		{
			// 逻辑核心掩码

			XAUDIO2_PROCESSOR processor = XAUDIO2_DEFAULT_PROCESSOR;
			if (WindowsVersion::Is10Build18362())
			{
				// 从 Windows 10 1903 开始支持
				processor = XAUDIO2_USE_DEFAULT_PROCESSOR;
			}

			// Windows NT 版本

			DWORD version = NTDDI_WIN10;
			// TODO: 等 NI 分支或者之后的分支出来要记得更新一下，也不知道 SB 微软为什么这么设计……
			if (WindowsVersion::Is11Build22000()) 
			{
				// NTDDI_WIN10_MN Windows Server 2022 preview
				// NTDDI_WIN10_FE Windows Server 2022 (20348/20349)
				// 所以直接跳到了 Windows 11 21H2
				version = NTDDI_WIN10_CO;
			}
			if (WindowsVersion::Is10Build19041())
			{
				// NTDDI_WIN10_VB 难以确定，可能是 Windows 10 1909，也可能 Windows 10 2004/20H1
				// 考虑到 1903/1909 共用一个内核，2004/20H2/21H1/21H2/22H2 共用一个内核
				// 为了保险起见，视其为 Windows 10 2004/20H1
				version = NTDDI_WIN10_VB;
			}
			else if (WindowsVersion::Is10Build18362())
			{
				// Windows 10 1903/19H1
				version = NTDDI_WIN10_19H1;
			}
			else
			{
				// Windows 10 1809
				version = NTDDI_WIN10_RS5;
			}

			// 终于开始创建了，好麻烦惹

			HRESULT hr = api_XAudio2CreateWithVersionInfo(ppXAudio2, 0, processor, version);
			if (SUCCEEDED(hr))
			{
				return hr;
			}
		}

		// 标准 API

		if (api_XAudio2Create)
		{
			// 从 Windows 10 1903/19H1 开始支持
			// 或者通过 xaudio2_9redist 支持
			HRESULT hr = api_XAudio2Create(ppXAudio2, 0, XAUDIO2_USE_DEFAULT_PROCESSOR);
			if (FAILED(hr))
			{
				// 回落到老版本的参数
				hr = api_XAudio2Create(ppXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
			}
			assert(SUCCEEDED(hr));
			return hr;
		}
		else
		{
			assert(false);
			return E_NOTIMPL;
		}
	}

	XAudio2::XAudio2()
	{
		std::array<std::pair<std::wstring_view, bool>, 3> dll_list = {
			std::make_pair<std::wstring_view, bool>(L"xaudio2_9.dll", true),
			std::make_pair<std::wstring_view, bool>(L"xaudio2_9redist.dll", false),
			std::make_pair<std::wstring_view, bool>(L"xaudio2_8.dll", true),
		};
		for (auto const& v : dll_list)
		{
			DWORD flags = 0;
			if (v.second)
			{
				flags |= LOAD_LIBRARY_SEARCH_SYSTEM32;
			}
			if (HMODULE dll = LoadLibraryExW(v.first.data(), NULL, flags))
			{
				dll_xaudio2 = dll;
				break;
			}
		}
		if (dll_xaudio2)
		{
			api_XAudio2CreateWithVersionInfo = (decltype(api_XAudio2CreateWithVersionInfo))
				GetProcAddress(dll_xaudio2, "XAudio2CreateWithVersionInfo");
			api_XAudio2Create = (decltype(api_XAudio2Create))
				GetProcAddress(dll_xaudio2, "XAudio2Create");
			assert(api_XAudio2Create);
		}
	}
	XAudio2::~XAudio2()
	{
		if (dll_xaudio2)
		{
			FreeLibrary(dll_xaudio2);
		}
		dll_xaudio2 = NULL;
		api_XAudio2CreateWithVersionInfo = NULL;
		api_XAudio2Create = NULL;
	}
}
