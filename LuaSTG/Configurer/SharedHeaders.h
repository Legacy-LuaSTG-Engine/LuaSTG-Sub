#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#include <sdkddkver.h>

#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <tuple>
#include <fstream>
#include <filesystem>

#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"

#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>

#include "utf8.hpp"
