#pragma once
#include "Common/f2dWindowsCommon.h"

// WM_USER = 0x0400 ( 1024)
// WM_APP  = 0x7FFF (32767)

constexpr UINT WM_MAIN_THREAD_DELEGATE = WM_USER + 0x0800;
constexpr UINT WM_SET_IME_ENABLE       = WM_USER + 0x0801;
