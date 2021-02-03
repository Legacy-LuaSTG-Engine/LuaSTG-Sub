#include "RenderDev/f2dTextureImpl.h"

#include "RenderDev/f2dRenderDeviceImpl.h"
#include "WICTextureLoader9.h"
#include <vector>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dTexture2DStatic::f2dTexture2DStatic(f2dRenderDevice* pDev, f2dStream* pStream, fuInt Width, fuInt Height, fBool HasMipmap)
	: m_Width(Width), m_Height(Height), m_pTex(NULL)
{
	if(!pStream)
		throw fcyException("f2dTexture2DStatic::f2dTexture2DStatic", "Param 'pStream' is null.");
	
	// 读取整个流
	std::vector<uint8_t> tData;
	tData.resize((size_t)pStream->GetLength());
	if(FCYFAILED(pStream->SetPosition(FCYSEEKORIGIN_BEG, 0)))
	{
		throw fcyException("f2dTexture2DStatic::f2dTexture2DStatic", "f2dStream::SetPosition Failed.");
	}
	if(FCYFAILED(pStream->ReadBytes(tData.data(), pStream->GetLength(), NULL)))
	{
		throw fcyException("f2dTexture2DStatic::f2dTexture2DStatic", "f2dStream::ReadBytes Failed.");
	}
	
	// 获取设备特性
	HRESULT tHR = 0;
	IDirect3DDevice9* d3d9 = (IDirect3DDevice9*)pDev->GetHandle();
	D3DCAPS9 caps;
	ZeroMemory(&caps, sizeof(D3DCAPS9));
	tHR = d3d9->GetDeviceCaps(&caps);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DStatic::f2dTexture2DStatic", "IDirect3DDevice9::GetDeviceCaps Failed.", tHR);
	
	// 检查设备特性
	uint32_t flags = DirectX::WIC_LOADER_DEFAULT;
	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) || (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) flags |= DirectX::WIC_LOADER_FIT_POW2;
	if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) flags |= DirectX::WIC_LOADER_MAKE_SQUARE;
	if (HasMipmap) flags |= DirectX::WIC_LOADER_MIP_AUTOGEN;
	
	// 加载图片
	tHR = DirectX::CreateWICTextureFromMemoryEx(
		d3d9,
		tData.data(), tData.size(),
		(caps.MaxTextureWidth < caps.MaxTextureHeight) ? caps.MaxTextureWidth : caps.MaxTextureHeight,
		0, D3DPOOL_MANAGED, flags,
		&m_pTex);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DStatic::f2dTexture2DStatic", "DirectX::CreateWICTextureFromMemoryEx Failed.", tHR);
	
	// 获取纹理尺寸
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
	tHR = m_pTex->GetLevelDesc(0, &desc);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DStatic::f2dTexture2DStatic", "IDirect3DTexture9::GetLevelDesc Failed.", tHR);
	m_Width = desc.Width;
	m_Height = desc.Height;
}

f2dTexture2DStatic::f2dTexture2DStatic(f2dRenderDevice* pDev, fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool HasMipmap)
	: m_Width(Width), m_Height(Height), m_pTex(NULL)
{
	if (!pMemory)
		throw fcyException("f2dTexture2DStatic::f2dTexture2DStatic", "Param 'pMemory' is null.");
	
	// 获取设备特性
	HRESULT tHR = 0;
	IDirect3DDevice9* d3d9 = (IDirect3DDevice9*)pDev->GetHandle();
	D3DCAPS9 caps;
	ZeroMemory(&caps, sizeof(D3DCAPS9));
	tHR = d3d9->GetDeviceCaps(&caps);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DStatic::f2dTexture2DStatic", "IDirect3DDevice9::GetDeviceCaps Failed.", tHR);
	
	// 检查设备特性
	uint32_t flags = DirectX::WIC_LOADER_DEFAULT;
	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) || (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) flags |= DirectX::WIC_LOADER_FIT_POW2;
	if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) flags |= DirectX::WIC_LOADER_MAKE_SQUARE;
	if (HasMipmap) flags |= DirectX::WIC_LOADER_MIP_AUTOGEN;
	
	// 加载图片
	tHR = DirectX::CreateWICTextureFromMemoryEx(
		d3d9,
		pMemory, (size_t)Size,
		(caps.MaxTextureWidth < caps.MaxTextureHeight) ? caps.MaxTextureWidth : caps.MaxTextureHeight,
		0, D3DPOOL_MANAGED, flags,
		&m_pTex);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DStatic::f2dTexture2DStatic", "DirectX::CreateWICTextureFromMemoryEx Failed.", tHR);
	
	// 获取纹理尺寸
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
	tHR = m_pTex->GetLevelDesc(0, &desc);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DStatic::f2dTexture2DStatic", "IDirect3DTexture9::GetLevelDesc Failed.", tHR);
	m_Width = desc.Width;
	m_Height = desc.Height;
}

f2dTexture2DStatic::~f2dTexture2DStatic()
{
	FCYSAFEKILL(m_pTex);
}

////////////////////////////////////////////////////////////////////////////////

f2dTexture2DDynamic::f2dTexture2DDynamic(f2dRenderDevice* pDev, fuInt Width, fuInt Height)
	: m_pParent(pDev), m_Width(Width), m_Height(Height)
{
	HRESULT tHR = ((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(
		m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, NULL);

	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DDevice9::CreateTexture Failed.", tHR);
	
	tHR = ((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(
		m_Width, m_Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_pCacheTex, NULL);
	
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DDevice9::CreateTexture Failed.", tHR);
	
	// 追加监听器
	m_pParent->AttachListener(this);
}

f2dTexture2DDynamic::f2dTexture2DDynamic(f2dRenderDevice* pDev, f2dStream* pStream, fuInt Width, fuInt Height)
	: m_pParent(pDev), m_Width(Width), m_Height(Height)
{
	if(!pStream)
		throw fcyException("f2dTexture2DDynamic::f2dTexture2DDynamic", "Param 'pStream' is null.");

	// 读取整个流
	std::vector<uint8_t> tData;
	tData.resize((size_t)pStream->GetLength());
	if(FCYFAILED(pStream->SetPosition(FCYSEEKORIGIN_BEG, 0)))
	{
		throw fcyException("f2dTexture2DStatic::f2dTexture2DStatic", "f2dStream::SetPosition Failed.");
	}
	if(FCYFAILED(pStream->ReadBytes(tData.data(), pStream->GetLength(), NULL)))
	{
		throw fcyException("f2dTexture2DStatic::f2dTexture2DStatic", "f2dStream::ReadBytes Failed.");
	}
	
	// 获取设备特性
	HRESULT tHR = 0;
	IDirect3DDevice9* d3d9 = (IDirect3DDevice9*)pDev->GetHandle();
	D3DCAPS9 caps;
	ZeroMemory(&caps, sizeof(D3DCAPS9));
	tHR = d3d9->GetDeviceCaps(&caps);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DDevice9::GetDeviceCaps Failed.", tHR);
	
	// 检查设备特性
	uint32_t flags = DirectX::WIC_LOADER_DEFAULT | DirectX::WIC_LOADER_FORCE_RGBA32;
	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) || (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) flags |= DirectX::WIC_LOADER_FIT_POW2;
	if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) flags |= DirectX::WIC_LOADER_MAKE_SQUARE;
	
	// 加载图片为Stage纹理
	tHR = DirectX::CreateWICTextureFromMemoryEx(
		d3d9,
		tData.data(), tData.size(),
		(caps.MaxTextureWidth < caps.MaxTextureHeight) ? caps.MaxTextureWidth : caps.MaxTextureHeight,
		D3DUSAGE_DYNAMIC, D3DPOOL_SYSTEMMEM, flags,
		&m_pCacheTex);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "DirectX::CreateWICTextureFromMemoryEx Failed.", tHR);
	
	// 获取纹理尺寸
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
	tHR = m_pCacheTex->GetLevelDesc(0, &desc);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DTexture9::GetLevelDesc Failed.", tHR);
	m_Width = desc.Width;
	m_Height = desc.Height;
	
	// 创建纹理
	tHR = ((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(
		m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, NULL);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DDevice9::CreateTexture Failed.", tHR);
	Upload();
	
	// 追加监听器
	m_pParent->AttachListener(this);
}

f2dTexture2DDynamic::f2dTexture2DDynamic(f2dRenderDevice* pDev, fcData pMemory, fLen Size, fuInt Width, fuInt Height)
	: m_pParent(pDev), m_Width(Width), m_Height(Height)
{
	if (!pMemory)
		throw fcyException("f2dTexture2DDynamic::f2dTexture2DDynamic", "Param 'pMemory' is null.");
	
	// 获取设备特性
	HRESULT tHR = 0;
	IDirect3DDevice9* d3d9 = (IDirect3DDevice9*)pDev->GetHandle();
	D3DCAPS9 caps;
	ZeroMemory(&caps, sizeof(D3DCAPS9));
	tHR = d3d9->GetDeviceCaps(&caps);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DDevice9::GetDeviceCaps Failed.", tHR);
	
	// 检查设备特性
	uint32_t flags = DirectX::WIC_LOADER_DEFAULT | DirectX::WIC_LOADER_FORCE_RGBA32;
	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) || (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) flags |= DirectX::WIC_LOADER_FIT_POW2;
	if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) flags |= DirectX::WIC_LOADER_MAKE_SQUARE;
	
	// 加载图片为Stage纹理
	tHR = DirectX::CreateWICTextureFromMemoryEx(
		d3d9,
		pMemory, (size_t)Size,
		(caps.MaxTextureWidth < caps.MaxTextureHeight) ? caps.MaxTextureWidth : caps.MaxTextureHeight,
		D3DUSAGE_DYNAMIC, D3DPOOL_SYSTEMMEM, flags,
		&m_pCacheTex);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "DirectX::CreateWICTextureFromMemoryEx Failed.", tHR);
	
	// 获取纹理尺寸
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
	tHR = m_pCacheTex->GetLevelDesc(0, &desc);
	if (FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DTexture9::GetLevelDesc Failed.", tHR);
	m_Width = desc.Width;
	m_Height = desc.Height;
	
	// 创建纹理
	tHR = ((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(
		m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, NULL);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DDynamic::f2dTexture2DDynamic", "IDirect3DDevice9::CreateTexture Failed.", tHR);
	Upload();
	
	// 追加监听器
	m_pParent->AttachListener(this);
}

f2dTexture2DDynamic::~f2dTexture2DDynamic()
{
	FCYSAFEKILL(m_pTex);
	FCYSAFEKILL(m_pCacheTex);

	// 移除监听器
	m_pParent->RemoveListener(this);
}

void f2dTexture2DDynamic::OnRenderDeviceLost()
{
	FCYSAFEKILL(m_pTex);
}

void f2dTexture2DDynamic::OnRenderDeviceReset()
{
	((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(
		m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, NULL);
}

fResult f2dTexture2DDynamic::Lock(fcyRect* pLockRect, fBool Discard, fuInt* Pitch, fData* pOut)
{
	if(pOut)
		*pOut = NULL;
	if(!m_pCacheTex)
		return FCYERR_INTERNALERR;
	if(Pitch)
		*Pitch = 0;


	RECT tRectToLock;
	D3DLOCKED_RECT tRectLocked;

	if(pLockRect)
	{
		tRectToLock.left = (int)pLockRect->a.x;
		tRectToLock.top = (int)pLockRect->a.y;
		tRectToLock.right = (int)pLockRect->b.x;
		tRectToLock.bottom = (int)pLockRect->b.y;
	}
	
	if(FAILED(m_pCacheTex->LockRect(0, &tRectLocked, (pLockRect ? &tRectToLock : NULL), Discard ? D3DLOCK_DISCARD : 0))) // D3DLOCK_NO_DIRTY_UPDATE
	{
		return FCYERR_INTERNALERR;
	}
	else
	{
		*Pitch = tRectLocked.Pitch;
		*pOut = ((fData)tRectLocked.pBits);

		return FCYERR_OK;
	}
}

fResult f2dTexture2DDynamic::Unlock()
{
	if(!m_pCacheTex)
		return FCYERR_INTERNALERR;

	if(FAILED(m_pCacheTex->UnlockRect(0)))
	{
		return FCYERR_INTERNALERR;
	}
	else
	{
		return FCYERR_OK;
	}
}

fResult f2dTexture2DDynamic::AddDirtyRect(fcyRect* pDirtyRect) {
	if(!m_pCacheTex)
		return FCYERR_INTERNALERR;
	RECT tRectToLock;
	if(pDirtyRect)
	{
		tRectToLock.left = (int)pDirtyRect->a.x;
		tRectToLock.top = (int)pDirtyRect->a.y;
		tRectToLock.right = (int)pDirtyRect->b.x;
		tRectToLock.bottom = (int)pDirtyRect->b.y;
	}
	m_pCacheTex->AddDirtyRect(pDirtyRect ? &tRectToLock : NULL);
	return FCYERR_OK;
}

fResult f2dTexture2DDynamic::Upload()
{
	if(!m_pCacheTex || !m_pTex)
		return FCYERR_INTERNALERR;

	if(FAILED(((IDirect3DDevice9*)m_pParent->GetHandle())->UpdateTexture(m_pCacheTex, m_pTex)))
	{
		return FCYERR_INTERNALERR;
	}
	else
	{
		return FCYERR_OK;
	}
}

////////////////////////////////////////////////////////////////////////////////

f2dTexture2DRenderTarget::f2dTexture2DRenderTarget(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize)
	: m_pParent(pDev), m_pSurface(NULL), m_Width(Width), m_Height(Height), m_bAutoResize(AutoResize), m_pTex(NULL)
{
	if(m_bAutoResize)
	{
		if(m_Width == 0)
			m_Width = pDev->GetBufferWidth();
		if(m_Height == 0)
			m_Height = pDev->GetBufferHeight();
	}

	HRESULT tHR = ((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, NULL);
	if(FAILED(tHR))
		throw fcyWin32COMException("f2dTexture2DRenderTarget::f2dTexture2DRenderTarget", "IDirect3DDevice9::CreateTexture Failed.", tHR);

	tHR = m_pTex->GetSurfaceLevel(0, &m_pSurface);

	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pTex);
		throw fcyWin32COMException("f2dTexture2DRenderTarget::f2dTexture2DRenderTarget", "IDirect3DTexture9::GetSurfaceLevel Failed.", tHR);
	}

	// 追加监听器
	m_pParent->AttachListener(this);
}

f2dTexture2DRenderTarget::~f2dTexture2DRenderTarget()
{
	FCYSAFEKILL(m_pSurface);
	FCYSAFEKILL(m_pTex);

	// 移除监听
	m_pParent->RemoveListener(this);
}

IDirect3DSurface9* f2dTexture2DRenderTarget::GetSurface()
{
	return m_pSurface;
}

void f2dTexture2DRenderTarget::OnRenderDeviceLost()
{
	FCYSAFEKILL(m_pSurface);
	FCYSAFEKILL(m_pTex);
}

void f2dTexture2DRenderTarget::OnRenderDeviceReset()
{
	if(m_bAutoResize)
	{
		m_Width = m_pParent->GetBufferWidth();
		m_Height = m_pParent->GetBufferHeight();
	}

	// 重置对象
	((IDirect3DDevice9*)m_pParent->GetHandle())->CreateTexture(m_Width, m_Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTex, NULL);
	if(m_pTex)
		m_pTex->GetSurfaceLevel(0, &m_pSurface);
}

////////////////////////////////////////////////////////////////////////////////

f2dDepthStencilSurfaceImpl::f2dDepthStencilSurfaceImpl(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize, fBool Discard)
	: m_pParent(pDev), m_pSurface(NULL), m_Width(Width), m_Height(Height), m_bAutoResize(AutoResize), m_bDiscard(Discard)
{
	if(m_bAutoResize)
	{
		if(m_Width == 0)
			m_Width = pDev->GetBufferWidth();
		if(m_Height == 0)
			m_Height = pDev->GetBufferHeight();
	}

	HRESULT tHR = ((IDirect3DDevice9*)m_pParent->GetHandle())->CreateDepthStencilSurface(m_Width, m_Height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, Discard, &m_pSurface, NULL);

	if(FAILED(tHR))
		throw fcyWin32COMException("f2dDepthStencilSurfaceImpl::f2dDepthStencilSurfaceImpl", "IDirect3DDevice9::CreateDepthStencilSurface Failed.", tHR);

	// 追加监听器
	m_pParent->AttachListener(this);
}

f2dDepthStencilSurfaceImpl::~f2dDepthStencilSurfaceImpl()
{
	FCYSAFEKILL(m_pSurface);

	// 释放监听器
	m_pParent->RemoveListener(this);
}

void f2dDepthStencilSurfaceImpl::OnRenderDeviceLost()
{
	FCYSAFEKILL(m_pSurface);
}

void f2dDepthStencilSurfaceImpl::OnRenderDeviceReset()
{
	if(m_bAutoResize)
	{
		m_Width = m_pParent->GetBufferWidth();
		m_Height = m_pParent->GetBufferHeight();
	}

	((IDirect3DDevice9*)m_pParent->GetHandle())->CreateDepthStencilSurface(m_Width, m_Height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, m_bDiscard, &m_pSurface, NULL);
}

void* f2dDepthStencilSurfaceImpl::GetHandle()
{
	return m_pSurface;
}

fuInt f2dDepthStencilSurfaceImpl::GetWidth()
{
	return m_Width;
}

fuInt f2dDepthStencilSurfaceImpl::GetHeight()
{
	return m_Height;
}
