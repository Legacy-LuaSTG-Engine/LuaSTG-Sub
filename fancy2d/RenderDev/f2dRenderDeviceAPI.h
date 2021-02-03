////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDeviceImpl.h
/// @brief fancy2D渲染设备接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyIO/fcyStream.h"
#include "fcyException.h"

#include "f2dRenderDevice.h"

#include <d3d9.h>
#include <d3dx9.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief d3d9显式加载
////////////////////////////////////////////////////////////////////////////////
class f2dRenderDeviceAPI
{
private:
    typedef HRESULT (WINAPI *pDLLEntry_D3DXSaveSurfaceToFileInMemory)(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);
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
	pDLLEntry_D3DXSaveSurfaceToFileInMemory m_Entry_pD3DXSaveSurfaceToFileInMemory;
	pDLLEntry_D3DXCreateEffectEx m_Entry_pD3DXCreateEffectEx;
public:
    HRESULT DLLEntry_D3DXSaveSurfaceToFileInMemory(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);
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
