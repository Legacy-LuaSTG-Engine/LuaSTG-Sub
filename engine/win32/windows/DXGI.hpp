#pragma once

namespace Platform
{
    class DXGI
    {
    public:
        static BOOL CheckFeatureSupportPresentAllowTearing(IDXGIFactory* pFactory);
        static HRESULT MakeSwapChainWindowAssociation(IDXGISwapChain* pSwapChain, UINT flags);
        static HRESULT SetSwapChainMaximumFrameLatency(IDXGISwapChain* pSwapChain, UINT MaxLatency, HANDLE* pEvent);
        static HRESULT SetDeviceMaximumFrameLatency(IDXGISwapChain* pSwapChain, UINT MaxLatency);
    };
}
