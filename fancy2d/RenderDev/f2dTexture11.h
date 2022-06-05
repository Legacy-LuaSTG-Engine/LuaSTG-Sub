#pragma once
#include "fcyIO/fcyStream.h"
#include "f2dRenderDevice.h"
#include "Core/Graphics/Device_D3D11.hpp"

// Texture2D 静态或者动态
class f2dTexture2D11 :
	public fcyRefObjImpl<f2dTexture2D>
{
	friend class f2dRenderDevice11;
private:
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::Texture2D_D3D11> m_tex;

public: // 内部公开接口
	ID3D11Texture2D* GetResource() { return m_tex->GetResource(); }
	ID3D11ShaderResourceView* GetView() { return m_tex->GetView(); }

public: // 接口实现
	void* GetHandle() { return GetView(); };
	LuaSTG::Core::Graphics::ITexture2D* GetNativeTexture2D() { return *m_tex; }
	LuaSTG::Core::Graphics::IRenderTarget* GetNativeRenderTarget() { return nullptr; }
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return m_tex->isDynamic(); }
	fBool IsRenderTarget() { return false; }
	fuInt GetWidth() { return m_tex->getSize().x; }
	fuInt GetHeight() { return m_tex->getSize().y; }
	fBool IsPremultipliedAlpha() { return m_tex->isPremultipliedAlpha(); }
	void SetPremultipliedAlpha(fBool b) { m_tex->setPremultipliedAlpha(b); }

	fResult Update(fcyRect* dstRect, fData pData, fuInt pitch)
	{
		LuaSTG::Core::RectU rc(dstRect->a.x, dstRect->a.y, dstRect->b.x, dstRect->b.y);
		m_tex->uploadPixelData(rc, pData, pitch);
		return FCYERR_OK;
	}
protected:
	f2dTexture2D11(f2dRenderDevice* pDev, fuInt Width, fuInt Height);
	f2dTexture2D11(f2dRenderDevice* pDev, fcStr path, fBool bMipmap);
	~f2dTexture2D11() {}
};

/*

// RenderTarget
class f2dRenderTarget11 :
	public fcyRefObjImpl<f2dTexture2D>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDevice11;
private:
	f2dRenderDevice11* m_dev{ nullptr };
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::RenderTarget_D3D11> m_rt;
	fuInt m_Width = 0;
	fuInt m_Height = 0;
	fBool m_bAutoResize = true;
	fBool m_PremultipliedAlpha = true; // RenderTarget 默认其为预乘了 alpha 的纹理
public: // 内部公开接口
	ID3D11Texture2D* GetResource() { return static_cast<LuaSTG::Core::Graphics::Texture2D_D3D11*>(m_rt->getTexture())->GetResource(); }
	ID3D11RenderTargetView* GetRTView() { return m_rt->GetView(); }
	ID3D11ShaderResourceView* GetSRView() { return static_cast<LuaSTG::Core::Graphics::Texture2D_D3D11*>(m_rt->getTexture())->GetView(); }
public:
	void OnRenderSizeDependentResourcesCreate();

public:

	void* GetHandle() { return GetSRView(); }
	LuaSTG::Core::Graphics::ITexture2D* GetNativeTexture2D() { return m_rt->getTexture(); }
	LuaSTG::Core::Graphics::IRenderTarget* GetNativeRenderTarget() { return *m_rt; }
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return false; }
	fBool IsRenderTarget() { return true; }
	fuInt GetWidth() { return m_rt->getTexture()->getSize().x; }
	fuInt GetHeight() { return m_rt->getTexture()->getSize().y; }
	fBool IsPremultipliedAlpha() { return m_rt->getTexture()->isPremultipliedAlpha(); }
	void SetPremultipliedAlpha(fBool b) { m_rt->getTexture()->setPremultipliedAlpha(b); m_PremultipliedAlpha = b; }

	fResult Update(fcyRect* dstRect, fData pData, fuInt pitch) { return FCYERR_NOTSUPPORT; }
protected:
	f2dRenderTarget11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize);
	~f2dRenderTarget11();
};

// DepthStencil
class f2dDepthStencil11 :
	public fcyRefObjImpl<f2dDepthStencilSurface>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDevice11;
private:
	f2dRenderDevice11* m_dev{ nullptr };
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::DepthStencilBuffer_D3D11> m_ds;
	fuInt m_Width = 0;
	fuInt m_Height = 0;
	fBool m_bAutoResize = true;
public: // 内部公开接口
	ID3D11Texture2D* GetResource() { return m_ds->GetResource(); }
	ID3D11DepthStencilView* GetView() { return m_ds->GetView(); }

public:
	void OnRenderSizeDependentResourcesCreate();

public:
	void* GetHandle() { return GetView(); }
	LuaSTG::Core::Graphics::IDepthStencilBuffer* GetNativeDepthStencilBuffer() { return *m_ds; }
	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }

protected:
	f2dDepthStencil11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize);
	~f2dDepthStencil11();
};

//*/
