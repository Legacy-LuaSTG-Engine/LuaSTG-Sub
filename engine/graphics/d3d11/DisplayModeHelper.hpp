#pragma once
#include "d3d11/pch.h"

namespace d3d11 {
    bool getSwapChainNearestOutputFromWindow(IDXGISwapChain* swap_chain, HWND window, IDXGIOutput** out_output);
}
