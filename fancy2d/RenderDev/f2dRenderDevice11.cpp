#include "RenderDev/f2dRenderDevice11.h"
#include "RenderDev/f2dTexture11.h"
#include "Engine/f2dEngineImpl.h"

inline f2dDisplayMode display_mode_from(LuaSTG::Core::Graphics::DisplayMode const& mode)
{
	return f2dDisplayMode{
		.width = mode.width,
		.height = mode.height,
		.refresh_rate = {
			.numerator = mode.refresh_rate.numerator,
			.denominator = mode.refresh_rate.denominator,
		},
		.format = (fuInt)DXGI_FORMAT_B8G8R8A8_UNORM,
		.scanline_ordering = (fuInt)DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
		.scaling = (fuInt)DXGI_MODE_SCALING_UNSPECIFIED,
	};
}
inline LuaSTG::Core::Graphics::DisplayMode display_mode_to(f2dDisplayMode const& mode)
{
	return LuaSTG::Core::Graphics::DisplayMode{
		.width = mode.width,
		.height = mode.height,
		.refresh_rate = {
			.numerator = mode.refresh_rate.numerator,
			.denominator = mode.refresh_rate.denominator,
		},
		.format = LuaSTG::Core::Graphics::Format::B8G8R8A8_UNORM, // 未使用
	};
}

// 类主体

f2dRenderDevice11::f2dRenderDevice11(f2dEngineImpl* pEngine, f2dEngineRenderWindowParam* RenderWindowParam)
	: m_pEngine(pEngine)
{
	// 设备族

	m_pGraphicsDevice = dynamic_cast<LuaSTG::Core::Graphics::Device_D3D11*>(pEngine->GGetAppModel()->getDevice());

	dxgi_factory = m_pGraphicsDevice->GetDXGIFactory1();
	dxgi_adapter = m_pGraphicsDevice->GetDXGIAdapter1();

	d3d11_device = m_pGraphicsDevice->GetD3D11Device();
	d3d11_devctx = m_pGraphicsDevice->GetD3D11DeviceContext();
	
	// 交换链

	m_pSwapChain = dynamic_cast<LuaSTG::Core::Graphics::SwapChain_D3D11*>(pEngine->GGetAppModel()->getSwapChain());

	m_pGraphicsDevice->addEventListener(this);
	m_pSwapChain->addEventListener(this);
}
f2dRenderDevice11::~f2dRenderDevice11()
{
	m_pGraphicsDevice->removeEventListener(this);
	m_pSwapChain->removeEventListener(this);
}

void* f2dRenderDevice11::GetHandle() { return d3d11_device.Get(); }
fuInt f2dRenderDevice11::GetSupportedDeviceCount() { return m_pGraphicsDevice->GetAdapterNameArray().size(); }
fcStr f2dRenderDevice11::GetSupportedDeviceName(fuInt Index) { return m_pGraphicsDevice->GetAdapterNameArray()[Index].c_str(); }

// 事件

void f2dRenderDevice11::onDeviceCreate()
{
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderDeviceReset();
	}
}
void f2dRenderDevice11::onDeviceDestroy()
{
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderDeviceLost();
	}
}

void f2dRenderDevice11::onSwapChainCreate()
{
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesCreate();
	}
}
void f2dRenderDevice11::onSwapChainDestroy()
{
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesDestroy();
	}
}

fResult f2dRenderDevice11::AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority)
{
	if (Listener == NULL)
		return FCYERR_INVAILDPARAM;

	EventListenerNode node{
		.uuid = _iEventListenerUUID,
		.priority = Priority,
		.listener = Listener,
	};
	auto v = _setEventListeners.emplace(node);
	_iEventListenerUUID += 1;

	return FCYERR_OK;
}
fResult f2dRenderDevice11::RemoveListener(f2dRenderDeviceEventListener* Listener)
{
	int ifind = 0;
	for (auto it = _setEventListeners.begin(); it != _setEventListeners.end();)
	{
		if (it->listener == Listener)
		{
			it = _setEventListeners.erase(it);
			ifind += 1;
		}
		else
		{
			it++;
		}
	}

	return ifind > 0 ? FCYERR_OK : FCYERR_OBJNOTEXSIT;
}

// 创建资源

fResult f2dRenderDevice11::CreateTextureFromStream(f2dStream* pStream, fuInt, fuInt, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2D11(this, pStream, HasMipmap, IsDynamic);
	}
	catch (const std::bad_alloc&)
	{
		return FCYERR_OUTOFMEM;
	}
	catch (const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt, fuInt, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2D11(this, pMemory, Size, HasMipmap, IsDynamic);
	}
	catch (const std::bad_alloc&)
	{
		return FCYERR_OUTOFMEM;
	}
	catch (const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2D11(this, Width, Height);
	}
	catch (const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dRenderTarget11(this, Width, Height, AutoResize);
	}
	catch (const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool, fBool AutoResize, f2dDepthStencilSurface** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dDepthStencil11(this, Width, Height, AutoResize);
	}
	catch (const fcyException& e)
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

// 渲染附件

f2dTexture2D* f2dRenderDevice11::GetRenderTarget()
{
	return *m_RenderTarget;
}
f2dDepthStencilSurface* f2dRenderDevice11::GetDepthStencilSurface()
{
	return *m_DepthStencil;
}
fResult f2dRenderDevice11::SetRenderTargetAndDepthStencilSurface(f2dTexture2D* pTex, f2dDepthStencilSurface* pSurface)
{
	if (pTex && !pTex->IsRenderTarget())
		return FCYERR_INVAILDPARAM;

	if (*m_RenderTarget == pTex && *m_DepthStencil == pSurface)
		return FCYERR_OK;
	
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	m_RenderTarget = pTex; // 注意 pTex 可能是 NULL 代表需要重置为交换链的 RenderTarget
	m_DepthStencil = pSurface; // 注意 pSurface 可能是 NULL 代表需要重置为默认的 DepthStencil

	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtvs[1] = { m_pSwapChain->GetRTV() };
		ID3D11DepthStencilView* dsv = m_pSwapChain->GetDSV();
		if (*m_RenderTarget)
		{
			rtvs[0] = ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView();
		}
		if (*m_DepthStencil)
		{
			dsv = ((f2dDepthStencil11*)*m_DepthStencil)->GetView();
		}
		d3d11_devctx->OMSetRenderTargets(1, rtvs, dsv);
	}

	return FCYERR_OK;
}

// 交换链

fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel)
{
	m_pSwapChain->setVSync(VSync);
	if (!m_pSwapChain->setWindowMode(Width, Height, FlipModel))
	{
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::SetDisplayMode(f2dDisplayMode mode, fBool VSync)
{
	m_pSwapChain->setVSync(VSync);
	if (!m_pSwapChain->setExclusiveFullscreenMode(display_mode_to(mode)))
	{
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}
fuInt f2dRenderDevice11::GetBufferWidth() { return m_pSwapChain->getWidth(); }
fuInt f2dRenderDevice11::GetBufferHeight() { return m_pSwapChain->getHeight(); }

fResult f2dRenderDevice11::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL)
{
	return SetDisplayMode(Width, Height, 0, 0, Windowed, VSync, FlipModel);
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRateA, fuInt RefreshRateB, fBool Windowed, fBool VSync, fBool FlipModel)
{
	if (Windowed)
	{
		return SetDisplayMode(Width, Height, VSync, FlipModel);
	}
	else
	{
		LuaSTG::Core::Graphics::DisplayMode mode = {
			.width = Width,
			.height = Height,
			.refresh_rate = {
				.numerator = RefreshRateA,
				.denominator = RefreshRateB,
			},
			.format = LuaSTG::Core::Graphics::Format::B8G8R8A8_UNORM,
		};
		if (!m_pSwapChain->findBestMatchDisplayMode(mode))
			return FCYERR_INTERNALERR;
		return SetDisplayMode(display_mode_from(mode), VSync);
	}
}

// 纹理读写

fResult f2dRenderDevice11::SaveScreen(fcStrW path)
{
	if (!path)
		return FCYERR_INVAILDPARAM;
	if (!d3d11_devctx)
		return FCYERR_ILLEGAL;

	HRESULT hr = 0;

	ID3D11RenderTargetView* rtvs[1] = { NULL };
	d3d11_devctx->OMGetRenderTargets(1, rtvs, NULL);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rendertarget = rtvs[0];
	if (rtvs[0]) rtvs[0]->Release();

	Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource;
	d3d11_rendertarget->GetResource(&d3d11_resource);
	hr = gHR = DirectX::SaveWICTextureToFile(d3d11_devctx.Get(), d3d11_resource.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if (FAILED(hr))
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::SaveTexture(fcStrW path, f2dTexture2D* pTex)
{
	if (!path || !pTex)
		return FCYERR_INVAILDPARAM;
	if (!d3d11_devctx)
		return FCYERR_ILLEGAL;

	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_resource;
	if (pTex->IsRenderTarget())
	{
		f2dRenderTarget11* pRTex = dynamic_cast<f2dRenderTarget11*>(pTex);
		d3d11_resource = pRTex->GetResource();
	}
	else
	{
		f2dTexture2D11* pRTex = dynamic_cast<f2dTexture2D11*>(pTex);
		d3d11_resource = pRTex->GetResource();
	}
	hr = gHR = DirectX::SaveWICTextureToFile(d3d11_devctx.Get(), d3d11_resource.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if (FAILED(hr))
	{
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
