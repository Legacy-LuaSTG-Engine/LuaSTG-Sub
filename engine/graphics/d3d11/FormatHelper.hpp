#pragma once
#include "core/GraphicsFormat.hpp"
#include "d3d11/pch.h"

namespace d3d11 {
    DXGI_FORMAT toFormat(core::GraphicsFormat format);
}
