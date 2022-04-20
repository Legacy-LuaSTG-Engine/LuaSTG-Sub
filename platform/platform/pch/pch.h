#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#ifdef WINVER
#undef WINVER
#endif
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#include <sdkddkver.h>

#include <cassert>
#include <tuple>
#include <vector>
#include <string>
#include <string_view>

#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <ShellScalingApi.h>
