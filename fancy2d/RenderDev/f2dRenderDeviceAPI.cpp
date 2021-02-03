#include "f2dRenderDeviceAPI.h"

////////////////////////////////////////////////////////////////////////////////

f2dRenderDeviceAPI::f2dRenderDeviceAPI() :
	m_hModuleD3DX9(NULL),
	m_Entry_pD3DXCreateEffectEx(NULL)
{
	// 加载模块 D3DX9
	
	m_hModuleD3DX9 = LoadLibrary(L"D3DX9_43.dll");
	if(!m_hModuleD3DX9)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "LoadLibrary(D3DX9_43.dll) Failed.");
	
	// 获得入口 D3DX9
	
	m_Entry_pD3DXCreateEffectEx = (pDLLEntry_D3DXCreateEffectEx)GetProcAddress(m_hModuleD3DX9, "D3DXCreateEffectEx");
	if(!m_Entry_pD3DXCreateEffectEx)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(D3DXCreateEffectEx) Failed.");
}

f2dRenderDeviceAPI::~f2dRenderDeviceAPI()
{
}

HRESULT f2dRenderDeviceAPI::DLLEntry_D3DXCreateEffectEx(
	LPDIRECT3DDEVICE9               pDevice,
	LPCVOID                         pSrcData,
	UINT                            SrcDataLen,
	CONST D3DXMACRO*                pDefines,
	LPD3DXINCLUDE                   pInclude,
	LPCSTR                          pSkipConstants, 
	DWORD                           Flags,
	LPD3DXEFFECTPOOL                pPool,
	LPD3DXEFFECT*                   ppEffect,
	LPD3DXBUFFER*                   ppCompilationErrors)
{
	return m_Entry_pD3DXCreateEffectEx(pDevice, pSrcData, SrcDataLen, pDefines, pInclude,pSkipConstants, Flags, pPool, ppEffect, ppCompilationErrors);
}
