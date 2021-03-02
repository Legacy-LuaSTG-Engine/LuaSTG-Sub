#include "f2dRenderDeviceAPI.h"

////////////////////////////////////////////////////////////////////////////////

f2dRenderDeviceAPI::f2dRenderDeviceAPI() :
	m_hModuleD3DX9(NULL),
	m_Entry_pD3DXCreateEffectEx(NULL)
{
}

f2dRenderDeviceAPI::~f2dRenderDeviceAPI()
{
	if (m_hModuleD3DX9)
		FreeLibrary(m_hModuleD3DX9);
	m_hModuleD3DX9 = NULL;
	m_Entry_pD3DXCreateEffectEx = NULL;
}

void f2dRenderDeviceAPI::_lazyLoad()
{
	if (!m_hModuleD3DX9)
	{
		m_hModuleD3DX9 = LoadLibraryW(L"D3DX9_43.dll");
		if (m_hModuleD3DX9)
		{
			m_Entry_pD3DXCreateEffectEx = (pDLLEntry_D3DXCreateEffectEx)GetProcAddress(m_hModuleD3DX9, "D3DXCreateEffectEx");
		}
	}
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
	_lazyLoad();
	if (!m_Entry_pD3DXCreateEffectEx)
	{
		return D3DERR_NOTAVAILABLE;
	}
	return m_Entry_pD3DXCreateEffectEx(pDevice, pSrcData, SrcDataLen, pDefines, pInclude,pSkipConstants, Flags, pPool, ppEffect, ppCompilationErrors);
}
