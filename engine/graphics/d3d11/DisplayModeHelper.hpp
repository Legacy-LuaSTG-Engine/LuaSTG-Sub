#pragma once
#include "d3d11/pch.h"

namespace d3d11 {
    bool getSwapChainNearestOutputFromWindow(IDXGISwapChain* swap_chain, HWND window, IDXGIOutput** out_output);

    bool findBestDisplayMode(IDXGISwapChain1* swap_chain, HWND window, UINT target_width, UINT target_height, DXGI_MODE_DESC1& mode);
}
