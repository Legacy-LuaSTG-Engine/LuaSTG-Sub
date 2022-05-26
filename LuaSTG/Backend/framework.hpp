#pragma once

#include <string>
#include <string_view>
#include <atomic>
#include <algorithm>
#include <vector>
#include <unordered_map>

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d2d1_3.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "spdlog/spdlog.h"

#include "Tracy.hpp"
#include "TracyD3D11.hpp"
#include "TracyD3D11Ctx.hpp"

#include "platform/HResultChecker.hpp"
