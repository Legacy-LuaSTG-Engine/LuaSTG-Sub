////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDeviceImpl.h
/// @brief fancy2D渲染设备接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyException.h"
#include "fcyIO/fcyStream.h"
#include "f2dRenderDevice.h"
#include "Common/f2dWindowsCommon.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief d3d9显式加载
////////////////////////////////////////////////////////////////////////////////
class f2dRenderDeviceAPI
{
private:
    typedef HRESULT (WINAPI *pDLLEntry_D3DXCreateEffectEx)(
        LPDIRECT3DDEVICE9               pDevice,
        LPCVOID                         pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
private:
	HMODULE m_hModuleD3DX9;
	pDLLEntry_D3DXCreateEffectEx m_Entry_pD3DXCreateEffectEx;
    void _lazyLoad();
public:
    HRESULT DLLEntry_D3DXCreateEffectEx(
        LPDIRECT3DDEVICE9               pDevice,
        LPCVOID                         pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
public:
    f2dRenderDeviceAPI();
    ~f2dRenderDeviceAPI();
};
