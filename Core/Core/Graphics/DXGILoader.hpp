#pragma once

namespace RuntimeLoader
{
	struct DXGI
	{
		HMODULE dll_dxgi{ NULL };
		decltype(CreateDXGIFactory)* api_CreateDXGIFactory{ NULL };
		decltype(CreateDXGIFactory1)* api_CreateDXGIFactory1{ NULL };
		decltype(CreateDXGIFactory2)* api_CreateDXGIFactory2{ NULL };

		HRESULT CreateFactory(REFIID riid, void** ppFactory)
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

		static BOOL CheckFeatureSupportPresentAllowTearing(IDXGIFactory* pFactory)
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
		static HRESULT MakeSwapChainWindowAssociation(IDXGISwapChain* pSwapChain, UINT flags)
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
		static HRESULT SetDeviceMaximumFrameLatency(IDXGISwapChain* pSwapChain, UINT MaxLatency)
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

		DXGI()
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
		~DXGI()
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
	};
}
