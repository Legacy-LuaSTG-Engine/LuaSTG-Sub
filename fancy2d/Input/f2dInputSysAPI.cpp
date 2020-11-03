#ifndef _M_ARM

#include "Input/f2dInputSysAPI.h"

////////////////////////////////////////////////////////////////////////////////

f2dInputSysAPI::f2dInputSysAPI()
	: m_hModule(NULL),
	m_Entry_pDirectInput8Create(NULL)
{
	// 加载模块
	m_hModule = LoadLibrary(L"DInput8.dll");
	if(!m_hModule)
		throw fcyWin32Exception("f2dInputSysAPI::f2dInputSysAPI", "LoadLibrary Failed.");

	// 获得入口
	m_Entry_pDirectInput8Create = (pDLLEntry_DirectInput8Create)GetProcAddress(m_hModule, "DirectInput8Create");
	if(!m_Entry_pDirectInput8Create)
		throw fcyWin32Exception("f2dInputSysAPI::f2dInputSysAPI", "GetProcAddress Failed.");
}

f2dInputSysAPI::~f2dInputSysAPI()
{
}

HRESULT f2dInputSysAPI::DLLEntry_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
	return m_Entry_pDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

#endif
