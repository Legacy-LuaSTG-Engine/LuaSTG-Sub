#pragma once

#include <algorithm>

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "spdlog/spdlog.h"

#include "Tracy.hpp"
#include "TracyD3D11.hpp"
#include "TracyD3D11Ctx.hpp"

#include "platform/HResultChecker.hpp"
