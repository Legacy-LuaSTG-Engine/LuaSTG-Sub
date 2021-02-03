////////////////////////////////////////////////////////////////////////////////
/// @file  f2dTextureImpl.h
/// @brief fancy2D 纹理包装
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "RenderDev/f2dRenderDeviceAPI.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D静态二维纹理
////////////////////////////////////////////////////////////////////////////////
class f2dTexture2DStatic :
	public fcyRefObjImpl<f2dTexture2D>
{
	friend class f2dRenderDeviceImpl;
private:
	IDirect3DTexture9* m_pTex;
	fuInt m_Width;
	fuInt m_Height;
public: // 接口实现
	void* GetHandle() { return m_pTex; }
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return false; }
	fBool IsRenderTarget() { return false; }

	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }

	fResult Lock(fcyRect* pLockRect, fBool Discard, fuInt* Pitch, fData* pOut) { return FCYERR_NOTSUPPORT; }

	fResult Unlock() { return FCYERR_NOTSUPPORT; }
	fResult AddDirtyRect(fcyRect* pDirtyRect) { return FCYERR_NOTSUPPORT; }
	fResult Upload() { return FCYERR_NOTSUPPORT; }
protected:
	f2dTexture2DStatic(f2dRenderDevice* pDev, f2dStream* pStream, fuInt Width, fuInt Height, fBool HasMipmap);
	f2dTexture2DStatic(f2dRenderDevice* pDev, fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool HasMipmap);
	~f2dTexture2DStatic();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D动态二维纹理
////////////////////////////////////////////////////////////////////////////////
class f2dTexture2DDynamic :
	public fcyRefObjImpl<f2dTexture2D>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDeviceImpl;
private:
	f2dRenderDevice* m_pParent = nullptr;

	IDirect3DTexture9* m_pTex = nullptr;
	IDirect3DTexture9* m_pCacheTex = nullptr;
	fuInt m_Width = 0;
	fuInt m_Height = 0;
public: // 设备丢失
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	void* GetHandle() { return m_pTex; } // 记住别手贱把这里改成 m_pCacheTex
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return true; }
	fBool IsRenderTarget() { return false; }

	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }

	fResult Lock(fcyRect* pLockRect, fBool Discard, fuInt* Pitch, fData* pOut);
	fResult Unlock();
	fResult AddDirtyRect(fcyRect* pDirtyRect);
	fResult Upload();
protected:
	f2dTexture2DDynamic(f2dRenderDevice* pDev, fuInt Width, fuInt Height);
	f2dTexture2DDynamic(f2dRenderDevice* pDev, f2dStream* pStream, fuInt Width, fuInt Height);
	f2dTexture2DDynamic(f2dRenderDevice* pDev, fcData pMemory, fLen Size, fuInt Width, fuInt Height);
	~f2dTexture2DDynamic();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D二维渲染目标
////////////////////////////////////////////////////////////////////////////////
class f2dTexture2DRenderTarget :
	public fcyRefObjImpl<f2dTexture2D>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDeviceImpl;
private:
	f2dRenderDevice* m_pParent;

	IDirect3DTexture9* m_pTex;
	fuInt m_Width;
	fuInt m_Height;

	fBool m_bAutoResize;

	IDirect3DSurface9* m_pSurface;
public: // 内部公开接口
	IDirect3DSurface9* GetSurface();
public: // 设备丢失
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	void* GetHandle() { return m_pTex; }
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return false; }
	fBool IsRenderTarget() { return true; }

	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }

	fResult Lock(fcyRect* pLockRect, fBool Discard, fuInt* Pitch, fData* pOut) { return FCYERR_NOTSUPPORT; }

	fResult Unlock() { return FCYERR_NOTSUPPORT; }
	fResult AddDirtyRect(fcyRect* pDirtyRect) { return FCYERR_NOTSUPPORT; }
	fResult Upload() { return FCYERR_NOTSUPPORT; }
protected:
	f2dTexture2DRenderTarget(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize);
	~f2dTexture2DRenderTarget();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D深度模板缓冲
////////////////////////////////////////////////////////////////////////////////
class f2dDepthStencilSurfaceImpl :
	public fcyRefObjImpl<f2dDepthStencilSurface>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDeviceImpl;
private:
	f2dRenderDevice* m_pParent;
	
	IDirect3DSurface9* m_pSurface;
	fuInt m_Width;
	fuInt m_Height;

	fBool m_bAutoResize;
	fBool m_bDiscard;
public: // 设备丢失
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	void* GetHandle();
	fuInt GetWidth();
	fuInt GetHeight();
protected:
	f2dDepthStencilSurfaceImpl(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize, fBool Discard);
	~f2dDepthStencilSurfaceImpl();
};
