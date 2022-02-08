#include "RenderDev/f2dTexture11.h"
#include "RenderDev/f2dRenderDevice11.h"
#include <spdlog/spdlog.h>

// Texture2D

f2dTexture2D11::f2dTexture2D11(f2dRenderDevice* pDev, fuInt Width, fuInt Height)
	: m_pParent((f2dRenderDevice11*)pDev)
	, m_Width(Width)
	, m_Height(Height)
	, m_Dynamic(true)
{
	OnRenderDeviceReset();
	m_pParent->AttachListener(this);
}
f2dTexture2D11::f2dTexture2D11(f2dRenderDevice* pDev, f2dStream* pStream, fBool bMipmap, fBool bDynamic)
	: m_pParent((f2dRenderDevice11*)pDev)
	, m_Dynamic(bDynamic)
{
	if (!pStream)
	{
		throw fcyException("f2dTexture2D11::f2dTexture2D11", "stream is null");
	}
	
	// 读取整个流
	std::vector<uint8_t> tData;
	tData.resize((size_t)pStream->GetLength());
	if (FCYFAILED(pStream->SetPosition(FCYSEEKORIGIN_BEG, 0)))
	{
		throw fcyException("f2dTexture2D11::f2dTexture2D11", "f2dStream::SetPosition Failed.");
	}
	if (FCYFAILED(pStream->ReadBytes(tData.data(), pStream->GetLength(), NULL)))
	{
		throw fcyException("f2dTexture2D11::f2dTexture2D11", "f2dStream::ReadBytes Failed.");
	}

	f2dTexture2D11FromMemory((fData)tData.data(), (fLen)tData.size(), bMipmap, bDynamic);

	m_pParent->AttachListener(this);
}
f2dTexture2D11::f2dTexture2D11(f2dRenderDevice* pDev, fcData pMemory, fLen Size, fBool bMipmap, fBool bDynamic)
	: m_pParent((f2dRenderDevice11*)pDev)
	, m_Dynamic(bDynamic)
{
	f2dTexture2D11FromMemory(pMemory, Size, bMipmap, bDynamic);
	m_pParent->AttachListener(this);
}
f2dTexture2D11::~f2dTexture2D11()
{
	m_pParent->RemoveListener(this);
}

void f2dTexture2D11::f2dTexture2D11FromMemory(fcData pMemory, fLen Size, fBool bMipmap, fBool bDynamic)
{
	HRESULT hr = 0;
	ID3D11Device* dev = (ID3D11Device*)m_pParent->GetHandle();
	dev->GetImmediateContext(&d3d11_devctx);

	// 加载图片
	Microsoft::WRL::ComPtr<ID3D11Resource> res;
	hr = gHR = DirectX::CreateWICTextureFromMemoryEx(
		dev, bMipmap ? d3d11_devctx.Get() : NULL,
		(uint8_t*)pMemory, (size_t)Size,
		0,
		D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
		DirectX::WIC_LOADER_DEFAULT,
		&res, &d3d11_srv);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dTexture2D11::f2dTexture2D11", "DirectX::CreateWICTextureFromMemoryEx Failed.", hr); // 假装这里是 f2dTexture2D11::f2dTexture2D11
	}

	// 转换类型
	hr = gHR = res.As(&d3d11_texture2d);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dTexture2D11::f2dTexture2D11", "ID3D11Resource::QueryInterface Failed.", hr); // 假装这里是 f2dTexture2D11::f2dTexture2D11
	}

	// 获取图片尺寸
	D3D11_TEXTURE2D_DESC t2dinfo = {};
	d3d11_texture2d->GetDesc(&t2dinfo);
	m_Width = t2dinfo.Width;
	m_Height = t2dinfo.Height;

	m_Handle.m_Tex2D = d3d11_texture2d.Get();
	m_Handle.m_SRV = d3d11_srv.Get();
}
void f2dTexture2D11::OnRenderDeviceLost()
{
	d3d11_devctx.Reset();
	d3d11_texture2d.Reset();
	d3d11_srv.Reset();
	m_Handle.m_Tex2D = NULL;
	m_Handle.m_SRV = NULL;
}
void f2dTexture2D11::OnRenderDeviceReset()
{
	ID3D11Device* dev = (ID3D11Device*)m_pParent->GetHandle();
	dev->GetImmediateContext(&d3d11_devctx);
	if (m_Dynamic)
	{
		HRESULT hr = 0;

		D3D11_TEXTURE2D_DESC texdef = {
			.Width = m_Width,
			.Height = m_Height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1,.Quality = 0},
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_SHADER_RESOURCE,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
		};
		hr = gHR = dev->CreateTexture2D(&texdef, NULL, &d3d11_texture2d);
		if (FAILED(hr))
		{
			throw fcyWin32COMException("f2dTexture2D11::f2dTexture2D11", "ID3D11Device::CreateTexture2D Failed.", hr); // 假装这里是 f2dTexture2D11::f2dTexture2D11
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC viewdef = {
			.Format = texdef.Format,
			.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
			.Texture2D = D3D11_TEX2D_SRV{.MostDetailedMip = 0,.MipLevels = 1,},
		};
		hr = gHR = dev->CreateShaderResourceView(d3d11_texture2d.Get(), &viewdef, &d3d11_srv);
		if (FAILED(hr))
		{
			throw fcyWin32COMException("f2dTexture2D11::f2dTexture2D11", "ID3D11Device::CreateShaderResourceView Failed.", hr); // 假装这里是 f2dTexture2D11::f2dTexture2D11
		}

		m_Handle.m_Tex2D = d3d11_texture2d.Get();
		m_Handle.m_SRV = d3d11_srv.Get();
	}
	else
	{
		// TODO: 完了，静态图片怎么办？？？要不丢给 LuaSTG 做，这里是 fancy2d 照顾不了太多
	}
}

fResult f2dTexture2D11::Update(fcyRect* dstRect, fData pData, fuInt pitch)
{
	if (!m_Dynamic)
	{
		return FCYERR_ILLEGAL;
	}
	if (!d3d11_devctx || !d3d11_texture2d)
	{
		return FCYERR_INTERNALERR;
	}
	D3D11_BOX box = {
		.left = (UINT)dstRect->a.x,
		.top = (UINT)dstRect->a.y,
		.front = 0,
		.right = (UINT)dstRect->b.x,
		.bottom = (UINT)dstRect->b.y,
		.back = 1,
	};
	d3d11_devctx->UpdateSubresource(d3d11_texture2d.Get(), 0, &box, pData, pitch, 0);
	return FCYERR_OK;
}

// RenderTarget

f2dRenderTarget11::f2dRenderTarget11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize)
	: m_pParent((f2dRenderDevice11*)pDev)
	, m_Width(Width)
	, m_Height(Height)
	, m_bAutoResize(AutoResize)
{
	OnRenderSizeDependentResourcesCreate();
	m_pParent->AttachListener(this);
}
f2dRenderTarget11::~f2dRenderTarget11()
{
	m_pParent->RemoveListener(this);
}

void f2dRenderTarget11::OnRenderDeviceLost()
{
	OnRenderSizeDependentResourcesDestroy();
}
void f2dRenderTarget11::OnRenderDeviceReset()
{
	OnRenderSizeDependentResourcesCreate();
}
void f2dRenderTarget11::OnRenderSizeDependentResourcesDestroy()
{
	d3d11_texture2d.Reset();
	d3d11_srv.Reset();
	d3d11_rtv.Reset();
	m_Handle.m_Tex2D = NULL;
	m_Handle.m_SRV = NULL;
	m_Handle.m_RTV = NULL;
}
void f2dRenderTarget11::OnRenderSizeDependentResourcesCreate()
{
	if (m_bAutoResize)
	{
		m_Width = m_pParent->GetBufferWidth();
		m_Height = m_pParent->GetBufferHeight();
	}
	else if (m_Width == 0 || m_Height == 0)
	{
		throw fcyException("f2dRenderTarget11::f2dRenderTarget11", "width or height is 0.");
	}
	
	HRESULT hr = 0;
	ID3D11Device* dev = (ID3D11Device*)m_pParent->GetHandle();

	D3D11_TEXTURE2D_DESC tex2ddef = {
		.Width = m_Width,
		.Height = m_Height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
		.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1,.Quality = 0,},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
	};
	hr = gHR = dev->CreateTexture2D(&tex2ddef, NULL, &d3d11_texture2d);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dRenderTarget11::f2dRenderTarget11", "ID3D11Device::CreateTexture2D failed.", hr);
	}
	
	D3D11_RENDER_TARGET_VIEW_DESC rtvdef = {
		.Format = tex2ddef.Format,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D = D3D11_TEX2D_RTV{.MipSlice = 0,},
	};
	hr = gHR = dev->CreateRenderTargetView(d3d11_texture2d.Get(), &rtvdef, &d3d11_rtv);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dRenderTarget11::f2dRenderTarget11", "ID3D11Device::CreateRenderTargetView failed.", hr);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewdef = {
		.Format = tex2ddef.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = D3D11_TEX2D_SRV{.MostDetailedMip = 0,.MipLevels = 1,},
	};
	hr = gHR = dev->CreateShaderResourceView(d3d11_texture2d.Get(), &viewdef, &d3d11_srv);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dRenderTarget11::f2dRenderTarget11", "ID3D11Device::CreateShaderResourceView Failed.", hr);
	}

	m_Handle.m_Tex2D = d3d11_texture2d.Get();
	m_Handle.m_SRV = d3d11_srv.Get();
	m_Handle.m_RTV = d3d11_rtv.Get();
}

// DepthStencil

f2dDepthStencil11::f2dDepthStencil11(f2dRenderDevice* pDev, fuInt Width, fuInt Height, fBool AutoResize)
	: m_pParent((f2dRenderDevice11*)pDev)
	, m_Width(Width)
	, m_Height(Height)
	, m_bAutoResize(AutoResize)
{
	OnRenderSizeDependentResourcesCreate();
	m_pParent->AttachListener(this);
}
f2dDepthStencil11::~f2dDepthStencil11()
{
	m_pParent->RemoveListener(this);
}

void f2dDepthStencil11::OnRenderDeviceLost()
{
	OnRenderSizeDependentResourcesDestroy();
}
void f2dDepthStencil11::OnRenderDeviceReset()
{
	OnRenderSizeDependentResourcesCreate();
}
void f2dDepthStencil11::OnRenderSizeDependentResourcesDestroy()
{
	d3d11_texture2d.Reset();
	d3d11_dsv.Reset();
	m_Handle.m_Tex2D = NULL;
	m_Handle.m_DSV = NULL;
}
void f2dDepthStencil11::OnRenderSizeDependentResourcesCreate()
{
	if (m_bAutoResize)
	{
		m_Width = m_pParent->GetBufferWidth();
		m_Height = m_pParent->GetBufferHeight();
	}
	else if (m_Width == 0 || m_Height == 0)
	{
		throw fcyException("f2dDepthStencil11::f2dDepthStencil11", "width or height is 0.");
	}

	HRESULT hr = 0;
	ID3D11Device* dev = (ID3D11Device*)m_pParent->GetHandle();

	D3D11_TEXTURE2D_DESC tex2ddef = {
		.Width = m_Width,
		.Height = m_Height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1,.Quality = 0,},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
	};
	hr = gHR = dev->CreateTexture2D(&tex2ddef, NULL, &d3d11_texture2d);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dDepthStencil11::f2dDepthStencil11", "ID3D11Device::CreateTexture2D failed.", hr);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdef = {
		.Format = tex2ddef.Format,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Texture2D = D3D11_TEX2D_DSV{.MipSlice = 0,},
	};
	hr = gHR = dev->CreateDepthStencilView(d3d11_texture2d.Get(), &dsvdef, &d3d11_dsv);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dDepthStencil11::f2dDepthStencil11", "ID3D11Device::CreateDepthStencilView failed.", hr);
	}

	m_Handle.m_Tex2D = d3d11_texture2d.Get();
	m_Handle.m_DSV = d3d11_dsv.Get();
}
