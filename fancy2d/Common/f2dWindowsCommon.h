#pragma once

#ifndef __FANCY2D_WINDOWS_COMMON_H__
#define __FANCY2D_WINDOWS_COMMON_H__

// Windows
#include <Windows.h>
#include <wrl.h>
// Image
#include <wincodec.h>
// Direct3D11
#include <dxgi1_6.h>
#include <d3d11_4.h>
// Direct3D9
#include <d3d9.h>
#include "d3dx9.h"
// DirectInput
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Helper function
void debugPrintHRESULT(HRESULT hr, const wchar_t* message) noexcept;

#endif//__FANCY2D_WINDOWS_COMMON_H__
