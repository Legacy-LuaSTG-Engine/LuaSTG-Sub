////////////////////////////////////////////////////////////////////////////////
/// @file  f2dInputSysAPI.h
/// @brief fancy2D输入系统API
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief 动态加载DInput
////////////////////////////////////////////////////////////////////////////////
class f2dInputSysAPI
{
private:
	typedef HRESULT (WINAPI *pDLLEntry_DirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
private:
	HMODULE m_hModule;

	pDLLEntry_DirectInput8Create m_Entry_pDirectInput8Create;
public:
	HRESULT DLLEntry_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
public:
	f2dInputSysAPI();
	~f2dInputSysAPI();
};
