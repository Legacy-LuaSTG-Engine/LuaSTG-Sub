#pragma once

namespace Platform
{
    class Direct3D11
    {
	public:
		static HRESULT GetDeviceAdater(ID3D11Device* pDevice, IDXGIAdapter1** ppAdapter);
		static HRESULT GetDeviceFactory(ID3D11Device* pDevice, IDXGIFactory2** ppFactory);
    };
}
