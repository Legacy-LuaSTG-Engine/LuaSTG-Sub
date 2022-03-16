#pragma once
#include "fcyIO/fcyStream.h"
#include "f2dRenderDevice.h"
#include "Common/f2dStandardCommon.hpp"
#include "Common/f2dWindowsCommon.h"

// Texture2D 静态或者动态
class f2dTexture2D11 :
	public fcyRefObjImpl<f2dTexture2D>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDevice11;
private:
	struct NativeHandle
	{
		ID3D11ShaderResourceView* m_SRV = NULL;
		ID3D11Texture2D* m_Tex2D = NULL;
	};
	f2dRenderDevice11* m_pParent = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_devctx;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d11_srv;
	NativeHandle m_Handle;
	fuInt m_Width = 0;
	fuInt m_Height = 0;
	fBool m_Dynamic = false;
	fBool m_PremultipliedAlpha = false;
private:
	void f2dTexture2D11FromMemory(fcData pMemory, fLen Size, fBool bMipmap, fBool bDynamic);
public: // 内部公开接口
	ID3D11Texture2D* GetResource() { return d3d11_texture2d.Get(); }
	ID3D11ShaderResourceView* GetView() { return d3d11_srv.Get(); }
public: // 设备丢失
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	void* GetHandle() { return m_Handle.m_SRV; };
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return m_Dynamic; }
	fBool IsRenderTarget() { return false; }
	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }
	fBool IsPremultipliedAlpha() { return m_PremultipliedAlpha; }
	void SetPremultipliedAlpha(fBool b) { m_PremultipliedAlpha = b; }

	fResult Update(fcyRect* dstRect, fData pData, fuInt pitch);
protected:
	f2dTexture2D11(f2dRenderDevice* pDev, fuInt Width, fuInt Height);
	f2dTexture2D11(f2dRenderDevice* pDev, f2dStream* pStream, fBool bMipmap, fBool bDynamic);
	f2dTexture2D11(f2dRenderDevice* pDev, fcData pMemory, fLen Size, fBool bMipmap, fBool bDynamic);
	~f2dTexture2D11();
};

// RenderTarget
class f2dRenderTarget11 :
	public fcyRefObjImpl<f2dTexture2D>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDevice11;
private:
	struct NativeHandle
	{
		ID3D11ShaderResourceView* m_SRV = NULL;
		ID3D11Texture2D* m_Tex2D = NULL;
		ID3D11RenderTargetView* m_RTV = NULL;
	};
	f2dRenderDevice11* m_pParent = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3d11_srv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rtv;
	NativeHandle m_Handle;
	fuInt m_Width = 0;
	fuInt m_Height = 0;
	fBool m_bAutoResize = true;
	fBool m_PremultipliedAlpha = true; // RenderTarget 默认其为预乘了 alpha 的纹理
public: // 内部公开接口
	ID3D11Texture2D* GetResource() { return d3d11_texture2d.Get(); }
	ID3D11RenderTargetView* GetRTView() { return d3d11_rtv.Get(); }
	ID3D11ShaderResourceView* GetSRView() { return d3d11_srv.Get(); }
public: // 设备丢失
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
	void OnRenderSizeDependentResourcesDestroy();
	void OnRenderSizeDependentResourcesCreate();
public: // 接口实现
	void* GetHandle() { return m_Handle.m_SRV; }
	fuInt GetDimension() { return 2; }
	fBool IsDynamic() { return false; }
	fBool IsRenderTarget() { return true; }
	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }
	fBool IsPremultipliedAlpha() { return m_PremultipliedAlpha; }
	void SetPremultipliedAlpha(fBool b) { m_PremultipliedAlpha = b; }

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
	struct NativeHandle
	{
		ID3D11DepthStencilView* m_DSV = NULL;
		ID3D11Texture2D* m_Tex2D = NULL;
	};
	f2dRenderDevice11* m_pParent = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11_dsv;
	NativeHandle m_Handle;
	fuInt m_Width = 0;
	fuInt m_Height = 0;
	fBool m_bAutoResize = true;
public: // 内部公开接口
	ID3D11Texture2D* GetResource() { return d3d11_texture2d.Get(); }
	ID3D11DepthStencilView* GetView() { return d3d11_dsv.Get(); }
public: // 设备丢失
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
	void OnRenderSizeDependentResourcesDestroy();
	void OnRenderSizeDependentResourcesCreate();
public: // 接口实现
	void* GetHandle() { return m_Handle.m_DSV; }
	fuInt GetWidth() { return m_Width; }
	fuInt GetHeight() { return m_Height; }
protected:
	f2dDepthStencil11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize);
	~f2dDepthStencil11();
};
