#pragma once
#include "d3d11/pch.h"

namespace d3d11 {
    void logDeviceFeatureSupportDetails(ID3D11Device* device);
    void logDeviceFormatSupportDetails(ID3D11Device* device);
}
