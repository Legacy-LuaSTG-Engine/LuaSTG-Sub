#pragma once
#include "core/GraphicsDeviceManager.hpp"
#include "d3d11/pch.h"

namespace core {
    class GraphicsDeviceManagerDXGI {
    public:
        static bool refreshAndGetFactory(IDXGIFactory2** out_factory);
        static bool refreshAndFindAdapter(std::string_view name, IDXGIAdapter1** adapter, std::string* adapter_name = nullptr);
    };
}
