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
			UINT Flags,
			D3D_FEATURE_LEVEL TargetFeatureLevel,
			ID3D11Device** ppDevice,
			D3D_FEATURE_LEVEL* pFeatureLevel,
			ID3D11DeviceContext** ppImmediateContext);
		HRESULT CreateDeviceFromAdapter(
			IDXGIAdapter* pAdapter,
			UINT Flags,
			D3D_FEATURE_LEVEL TargetFeatureLevel,
			ID3D11Device** ppDevice,
			D3D_FEATURE_LEVEL* pFeatureLevel,
			ID3D11DeviceContext** ppImmediateContext);
		HRESULT CreateDeviceFromSoftAdapter(
			UINT Flags,
			D3D_FEATURE_LEVEL TargetFeatureLevel,
			ID3D11Device** ppDevice,
			D3D_FEATURE_LEVEL* pFeatureLevel,
			ID3D11DeviceContext** ppImmediateContext,
			D3D_DRIVER_TYPE* pType = nullptr);
	public:
		Direct3D11();
		~Direct3D11();
	};
}
