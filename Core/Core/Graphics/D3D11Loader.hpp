#pragma once

namespace RuntimeLoader
{
    struct Direct3D11
	{
		HMODULE dll_d3d11{ NULL };
		decltype(D3D11CreateDevice)* api_D3D11CreateDevice{ NULL };

		HRESULT CreateDevice(
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
		HRESULT CreateDevice(
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

		static HRESULT GetDeviceAdater(ID3D11Device* pDevice, IDXGIAdapter1** ppAdapter)
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
		static HRESULT GetDeviceFactory(ID3D11Device* pDevice, IDXGIFactory2** ppFactory)
		{
			assert(pDevice);
			assert(ppFactory);

			HRESULT hr = S_OK;

			Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;
			hr = GetDeviceAdater(pDevice, &dxgi_adapter);
			if (FAILED(hr)) return hr;

			hr = dxgi_adapter->GetParent(IID_PPV_ARGS(ppFactory));
			if (FAILED(hr)) return hr;

			return hr;
		}
		
		Direct3D11()
		{
			dll_d3d11 = LoadLibraryW(L"d3d11.dll");
			if (dll_d3d11)
			{
				api_D3D11CreateDevice = (decltype(api_D3D11CreateDevice))
					GetProcAddress(dll_d3d11, "D3D11CreateDevice");
			}
		}
		~Direct3D11()
		{
			if (dll_d3d11)
			{
				FreeLibrary(dll_d3d11);
			}
			dll_d3d11 = NULL;
			api_D3D11CreateDevice = NULL;
		}
	};
}
