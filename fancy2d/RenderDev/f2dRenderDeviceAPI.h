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
	// DX9
	typedef IDirect3D9* (WINAPI *pDLLEntry_Direct3DCreate9)(UINT SDKVersion);

	// D3DX9
	typedef HRESULT (WINAPI *pDLLEntry_D3DXSaveSurfaceToFileInMemory)(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);
	/*
	typedef HRESULT (WINAPI *pDLLEntry_D3DXCreateFontW)(
        LPDIRECT3DDEVICE9       pDevice,  
        INT                     Height,
        UINT                    Width,
        UINT                    Weight,
        UINT                    MipLevels,
        BOOL                    Italic,
        DWORD                   CharSet,
        DWORD                   OutputPrecision,
        DWORD                   Quality,
        DWORD                   PitchAndFamily,
        LPCWSTR                 pFaceName,
        LPD3DXFONT*             ppFont);
	*/
	typedef HRESULT (WINAPI *pDLLEntry_D3DXCreateTextureFromFileInMemoryEx)(
		LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
		DWORD                     Filter,
		DWORD                     MipFilter,
		D3DCOLOR                  ColorKey,
		D3DXIMAGE_INFO*           pSrcInfo,
		PALETTEENTRY*             pPalette,
		LPDIRECT3DTEXTURE9*       ppTexture);
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
	/*
	typedef HRESULT (WINAPI *pDLLEntry_D3DXCreateEffectFromFileExW)(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
		*/
private:
	HMODULE m_hModuleD3D9;
	HMODULE m_hModuleD3DX9;

	pDLLEntry_Direct3DCreate9 m_Entry_pDirect3DCreate9;

	pDLLEntry_D3DXSaveSurfaceToFileInMemory m_Entry_pD3DXSaveSurfaceToFileInMemory;
	// pDLLEntry_D3DXCreateFontW m_Entry_pD3DXCreateFontW;
	pDLLEntry_D3DXCreateTextureFromFileInMemoryEx m_Entry_pD3DXCreateTextureFromFileInMemoryEx;
	pDLLEntry_D3DXCreateEffectEx m_Entry_pD3DXCreateEffectEx;
	// pDLLEntry_D3DXCreateEffectFromFileExW m_Entry_pD3DXCreateEffectFromFileExW;
public:
	IDirect3D9* DLLEntry_Direct3DCreate9(UINT SDKVersion);

	HRESULT DLLEntry_D3DXSaveSurfaceToFileInMemory(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);
	/*
	HRESULT DLLEntry_D3DXCreateFontW(
        LPDIRECT3DDEVICE9       pDevice,  
        INT                     Height,
        UINT                    Width,
        UINT                    Weight,
        UINT                    MipLevels,
        BOOL                    Italic,
        DWORD                   CharSet,
        DWORD                   OutputPrecision,
        DWORD                   Quality,
        DWORD                   PitchAndFamily,
        LPCWSTR                 pFaceName,
        LPD3DXFONT*             ppFont);
	*/
	HRESULT DLLEntry_D3DXCreateTextureFromFileInMemoryEx(
		LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture);
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
	/*
	HRESULT DLLEntry_D3DXCreateEffectFromFileExW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
	*/
public:
	f2dRenderDeviceAPI();
	~f2dRenderDeviceAPI();
};
