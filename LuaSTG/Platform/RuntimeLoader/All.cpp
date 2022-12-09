#include "Platform/Shared.hpp"
#include "Platform/RuntimeLoader/DXGI.hpp"
#include "Platform/RuntimeLoader/Direct3D11.hpp"
#include "Platform/RuntimeLoader/Direct2D1.hpp"
#include "Platform/RuntimeLoader/DirectComposition.hpp"
#include "Platform/RuntimeLoader/DirectWrite.hpp"

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

	BOOL DXGI::CheckFeatureSupportPresentAllowTearing(IDXGIFactory* pFactory)
	{
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory = pFactory;
		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgi_factory5;
		if (SUCCEEDED(dxgi_factory.As(&dxgi_factory5)))
		{
			BOOL feature_supported = FALSE;
			if (SUCCEEDED(dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &feature_supported, sizeof(feature_supported))))
			{
				return feature_supported;
			}
		}
		return FALSE;
	}
	HRESULT DXGI::MakeSwapChainWindowAssociation(IDXGISwapChain* pSwapChain, UINT flags)
	{
		assert(pSwapChain);

		HRESULT hr = S_OK;

		HWND hwnd = NULL;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain1;
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain1));
		if (SUCCEEDED(hr))
		{
			hr = pSwapChain1->GetHwnd(&hwnd);
			if (FAILED(hr)) return hr; // the swap chain might not be HWND-based
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC desc = {};
			hr = pSwapChain->GetDesc(&desc);
			if (FAILED(hr)) return hr;
			hwnd = desc.OutputWindow;
		}

		Microsoft::WRL::ComPtr<IDXGIDevice> pDxgiDevice;
		hr = pSwapChain->GetDevice(IID_PPV_ARGS(&pDxgiDevice));
		if (FAILED(hr)) return hr;

		Microsoft::WRL::ComPtr<IDXGIAdapter> pDxgiAdapter;
		hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
		if (FAILED(hr)) return hr;

		Microsoft::WRL::ComPtr<IDXGIFactory> pDxgiFactory;
		hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
		if (FAILED(hr)) return hr;

		hr = pDxgiFactory->MakeWindowAssociation(hwnd, flags);
		return hr;
	}
	HRESULT DXGI::SetDeviceMaximumFrameLatency(IDXGISwapChain* pSwapChain, UINT MaxLatency)
	{
		assert(pSwapChain);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device;
		hr = pSwapChain->GetDevice(IID_PPV_ARGS(&dxgi_device));
		if (FAILED(hr)) return hr;

		hr = dxgi_device->SetMaximumFrameLatency(MaxLatency);
		if (FAILED(hr)) return hr;

		return hr;
	}
}

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

	HRESULT Direct3D11::GetAdater(ID3D11Device* pDevice, IDXGIAdapter1** ppAdapter)
	{
		assert(pDevice);
		assert(ppAdapter);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
		hr = pDevice->QueryInterface(IID_PPV_ARGS(&dxgi_device));
		if (FAILED(hr)) return hr;

		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
		hr = dxgi_device->GetAdapter(&dxgi_adapter);
		if (FAILED(hr)) return hr;

		hr = dxgi_adapter->QueryInterface(IID_PPV_ARGS(ppAdapter));
		if (FAILED(hr)) return hr;

		return hr;
	}
	HRESULT Direct3D11::GetFactory(ID3D11Device* pDevice, IDXGIFactory2** ppFactory)
	{
		assert(pDevice);
		assert(ppFactory);

		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
		hr = GetAdater(pDevice, &dxgi_adapter);
		if (FAILED(hr)) return hr;

		hr = dxgi_adapter->GetParent(IID_PPV_ARGS(ppFactory));
		if (FAILED(hr)) return hr;

		return hr;
	}
}

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
