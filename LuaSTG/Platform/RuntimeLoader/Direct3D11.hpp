#pragma once

namespace Platform::RuntimeLoader
{
	class Direct3D11
	{
	private:
		HMODULE dll_d3d11{};
		decltype(D3D11CreateDevice)* api_D3D11CreateDevice{};
	public:
		HRESULT CreateDevice(
			D3D_DRIVER_TYPE DriverType,
			UINT Flags,
			D3D_FEATURE_LEVEL TargetFeatureLevel,
			ID3D11Device** ppDevice,
			D3D_FEATURE_LEVEL* pFeatureLevel,
			ID3D11DeviceContext** ppImmediateContext);
		HRESULT CreateDevice(
			IDXGIAdapter* pAdapter,
			UINT Flags,
			D3D_FEATURE_LEVEL TargetFeatureLevel,
			ID3D11Device** ppDevice,
			D3D_FEATURE_LEVEL* pFeatureLevel,
			ID3D11DeviceContext** ppImmediateContext);
	public:
		Direct3D11();
		~Direct3D11();
	public:
		static HRESULT GetAdater(ID3D11Device* pDevice, IDXGIAdapter1** ppAdapter);
		static HRESULT GetFactory(ID3D11Device* pDevice, IDXGIFactory2** ppFactory);
	};
}
