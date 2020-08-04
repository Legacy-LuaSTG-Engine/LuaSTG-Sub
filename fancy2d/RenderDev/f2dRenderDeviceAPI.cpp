#include "f2dRenderDeviceAPI.h"

#ifdef _M_ARM
#include "../extern/FreeImage/freeimage.h"
#pragma comment(lib, "freeimage.lib")
#endif

////////////////////////////////////////////////////////////////////////////////

f2dRenderDeviceAPI::f2dRenderDeviceAPI()
	: m_hModuleD3D9(NULL), m_hModuleD3DX9(NULL),
	m_Entry_pDirect3DCreate9(NULL),
	m_Entry_pD3DXSaveSurfaceToFileInMemory(NULL), /*m_Entry_pD3DXCreateFontW(NULL),*/ m_Entry_pD3DXCreateTextureFromFileInMemoryEx(NULL)
{
	// 加载模块 D3D9
	m_hModuleD3D9 = LoadLibrary(L"D3D9.dll");
	if(!m_hModuleD3D9)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "LoadLibrary(D3D9.dll) Failed.");
	// 获得入口 D3D9
	m_Entry_pDirect3DCreate9 = (pDLLEntry_Direct3DCreate9)GetProcAddress(m_hModuleD3D9, "Direct3DCreate9");
	if(!m_Entry_pDirect3DCreate9)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(Direct3DCreate9) Failed.");

#ifndef _M_ARM
	// 加载模块 D3DX9
	m_hModuleD3DX9 = LoadLibrary(L"D3DX9_43.dll");
	if(!m_hModuleD3DX9)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "LoadLibrary(D3DX9_43.dll) Failed.");
	// 获得入口 D3DX9
	m_Entry_pD3DXSaveSurfaceToFileInMemory = (pDLLEntry_D3DXSaveSurfaceToFileInMemory)GetProcAddress(m_hModuleD3DX9, "D3DXSaveSurfaceToFileInMemory");
	if(!m_Entry_pD3DXSaveSurfaceToFileInMemory)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(D3DXSaveSurfaceToFileInMemory) Failed.");
	/*
	m_Entry_pD3DXCreateFontW = (pDLLEntry_D3DXCreateFontW)GetProcAddress(m_hModuleD3DX9, "D3DXCreateFontW");
	if(!m_Entry_pD3DXCreateFontW)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(D3DXCreateFontW) Failed.");
	*/
	m_Entry_pD3DXCreateTextureFromFileInMemoryEx = (pDLLEntry_D3DXCreateTextureFromFileInMemoryEx)GetProcAddress(m_hModuleD3DX9, "D3DXCreateTextureFromFileInMemoryEx");
	if(!m_Entry_pD3DXCreateTextureFromFileInMemoryEx)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(D3DXCreateTextureFromFileInMemoryEx) Failed.");
	m_Entry_pD3DXCreateEffectEx = (pDLLEntry_D3DXCreateEffectEx)GetProcAddress(m_hModuleD3DX9, "D3DXCreateEffectEx");
	if(!m_Entry_pD3DXCreateEffectEx)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(D3DXCreateEffectEx) Failed.");
	/*
	m_Entry_pD3DXCreateEffectFromFileExW = (pDLLEntry_D3DXCreateEffectFromFileExW)GetProcAddress(m_hModuleD3DX9, "D3DXCreateEffectFromFileExW");
	if(!m_Entry_pD3DXCreateEffectFromFileExW)
		throw fcyWin32Exception("f2dRenderDeviceAPI::f2dRenderDeviceAPI", "GetProcAddress(D3DXCreateEffectFromFileExW) Failed.");
	*/
#endif
}

f2dRenderDeviceAPI::~f2dRenderDeviceAPI()
{
}

IDirect3D9* f2dRenderDeviceAPI::DLLEntry_Direct3DCreate9(UINT SDKVersion)
{
	return m_Entry_pDirect3DCreate9(SDKVersion);
}

HRESULT f2dRenderDeviceAPI::DLLEntry_D3DXSaveSurfaceToFileInMemory(
	LPD3DXBUFFER*             ppDestBuf,
	D3DXIMAGE_FILEFORMAT      DestFormat,
	LPDIRECT3DSURFACE9        pSrcSurface,
	CONST PALETTEENTRY*       pSrcPalette,
	CONST RECT*               pSrcRect)
{
#ifndef _M_ARM
	return m_Entry_pD3DXSaveSurfaceToFileInMemory(ppDestBuf, DestFormat, pSrcSurface, pSrcPalette, pSrcRect);
#else
	return E_NOTIMPL;
#endif
}

/*
HRESULT f2dRenderDeviceAPI::DLLEntry_D3DXCreateFontW(
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
	LPD3DXFONT*             ppFont)
{
	return m_Entry_pD3DXCreateFontW(pDevice, Height, Width, Weight, MipLevels, Italic, CharSet, OutputPrecision, Quality, PitchAndFamily, pFaceName, ppFont);
}
*/

HRESULT f2dRenderDeviceAPI::DLLEntry_D3DXCreateTextureFromFileInMemoryEx(
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
	LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifndef _M_ARM
	return m_Entry_pD3DXCreateTextureFromFileInMemoryEx(pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
#else
	if((Format != D3DFMT_X8R8G8B8 && Format != D3DFMT_A8R8G8B8) || pPalette)
		return E_NOTIMPL;
	if(!ppTexture || !pDevice)
		return E_POINTER;

	if((int)Width < 0)
		Width = 0;
	if((int)Height < 0)
		Height = 0;

	// FreeImage加载图片
	FIMEMORY* tFMem = FreeImage_OpenMemory((BYTE*)pSrcData, SrcDataSize);
	if(tFMem == NULL)
		return E_FAIL;

	FREE_IMAGE_FORMAT tFType = FreeImage_GetFileTypeFromMemory(tFMem);
	FIBITMAP * tFBitmap = FreeImage_LoadFromMemory(tFType, tFMem);
	
	// 转换格式
	FIBITMAP * tFBitmap_New = FreeImage_ConvertTo32Bits(tFBitmap);
	FreeImage_Unload(tFBitmap);
	tFBitmap = tFBitmap_New;
	tFBitmap_New = NULL;

	// 转换大小
	if(Width > 0 && Height > 0 && ((int)Width != FreeImage_GetWidth(tFBitmap) || (int)Height != FreeImage_GetHeight(tFBitmap)))
	{
		tFBitmap_New = FreeImage_Rescale(tFBitmap, Width, Height, FILTER_LANCZOS3);
		FreeImage_Unload(tFBitmap);
		tFBitmap = tFBitmap_New;
		tFBitmap_New = NULL;
	}
	else
	{
		Width = FreeImage_GetWidth(tFBitmap);
		Height = FreeImage_GetHeight(tFBitmap);
	}

	// 创建纹理
	IDirect3DTexture9* pRet = NULL;
	HRESULT tHR;
	if(FAILED(tHR = pDevice->CreateTexture(
		Width,
		Height,
		((int)MipLevels < 0 ? 0 : MipLevels),
		Usage,
		Format,
		Pool,
		&pRet,
		NULL
		)))
	{
		fCharW tBuf[512];
		swprintf_s(tBuf, L"Width = %u, Height = %u, MipLevel = %u \n HRESULT = %x", Width, Height, ((int)MipLevels < 0 ? 0 : MipLevels), tHR);
		MessageBox(0, tBuf, L"CreateTexture Failed", 0);

		FreeImage_Unload(tFBitmap);
		FreeImage_CloseMemory(tFMem);
		return tHR;
	}

	// 获得纹理数据的指针
	fuInt tSrcPitch = FreeImage_GetPitch(tFBitmap);
	fcyColor* pSrcPixels = (fcyColor*)(FreeImage_GetBits(tFBitmap) + tSrcPitch * (FreeImage_GetHeight(tFBitmap) - 1));

	// 锁定并复制数据
	D3DLOCKED_RECT pLockedRect;
	if(FAILED(pRet->LockRect(0, &pLockedRect, NULL, D3DLOCK_DISCARD)))
	{
		FreeImage_Unload(tFBitmap);
		FreeImage_CloseMemory(tFMem);
		return E_FAIL;
	}

	fcyColor* pDestPixel = (fcyColor*)pLockedRect.pBits;
	for(fuInt y = 0; y<Height; ++y)
	{
		for(fuInt x = 0; x<Width; ++x)
		{
			pDestPixel[x] = pSrcPixels[x];
		}

		pDestPixel = (fcyColor*)((fByte*)pDestPixel + pLockedRect.Pitch);
		pSrcPixels = (fcyColor*)((fByte*)pSrcPixels - tSrcPitch);
	}

	pRet->UnlockRect(0);

	// 产生mipmap
	pRet->GenerateMipSubLevels();

	// 输出数据
	if(pSrcInfo)
	{
		pSrcInfo->Depth = 32;
		pSrcInfo->Format = Format;
		pSrcInfo->ImageFileFormat = D3DXIFF_PNG;
		pSrcInfo->MipLevels = pRet->GetLevelCount();
		pSrcInfo->ResourceType = D3DRTYPE_TEXTURE;
		pSrcInfo->Width = Width;
		pSrcInfo->Height = Height;
	}

	// 销毁
	FreeImage_Unload(tFBitmap);
	FreeImage_CloseMemory(tFMem);

	// 返回
	*ppTexture = pRet;
	return S_OK;
#endif
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
#ifndef _M_ARM
	return m_Entry_pD3DXCreateEffectEx(pDevice, pSrcData, SrcDataLen, pDefines, pInclude,pSkipConstants, Flags, pPool, ppEffect, ppCompilationErrors);
#else
	return E_NOTIMPL;
#endif
}

/*
HRESULT f2dRenderDeviceAPI::DLLEntry_D3DXCreateEffectFromFileExW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors)
{
	return m_Entry_pD3DXCreateEffectFromFileExW(pDevice, pSrcFile, pDefines, pInclude, pSkipConstants, Flags, pPool, ppEffect, ppCompilationErrors);
}
*/
