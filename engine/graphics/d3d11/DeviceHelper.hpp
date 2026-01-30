#pragma once
#include "d3d11/pch.h"

namespace d3d11 {
    bool createDevice(
        IDXGIAdapter1* adapter,
        ID3D11Device** device, ID3D11DeviceContext** device_context, D3D_FEATURE_LEVEL* feature_level
    );

    bool createSoftwareDevice(
        ID3D11Device** device, ID3D11DeviceContext** device_context, D3D_FEATURE_LEVEL* feature_level,
        D3D_DRIVER_TYPE* driver_type
    );

    void logDeviceFeatureSupportDetails(ID3D11Device* device);

    void logDeviceFormatSupportDetails(ID3D11Device* device);
}
