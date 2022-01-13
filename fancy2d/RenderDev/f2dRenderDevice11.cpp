#include "RenderDev/f2dRenderDevice11.h"

#include <fcyMisc/fcyStringHelper.h>
#include <fcyOS/fcyDebug.h>

#include "RenderDev/f2dTexture11.h"

#include "Engine/f2dEngineImpl.h"

#ifdef max
#undef max // FUCK YOU!
#endif

// 类主体

f2dRenderDevice11::f2dRenderDevice11(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)
	: m_pEngine(pEngine)
	, m_CreateThreadID(GetCurrentThreadId())
	, swapchain_width(BackBufferWidth)
	, swapchain_height(BackBufferHeight)
	, swapchain_windowed(false) // 必须以窗口模式启动
	, swapchain_vsync(VSync)
{
	m_hWnd = (HWND)pEngine->GetMainWindow()->GetHandle();
	win32_window = m_hWnd;
	HRESULT hr = 0;

	// create DXGI basic components

	hr = gHR = CreateDXGIFactory1(IID_IDXGIFactory1, &dxgi_factory);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dRenderDevice11::f2dRenderDevice11", "CreateDXGIFactory1 failed.", hr);
	}
	hr = gHR = dxgi_factory->EnumAdapters1(0, &dxgi_adapter);
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dRenderDevice11::f2dRenderDevice11", "IDXGIFactory1::EnumAdapters1 failed.", hr);
	}
	DXGI_ADAPTER_DESC1 adapter_info = {};
	hr = gHR = dxgi_adapter->GetDesc1(&adapter_info);
	if (SUCCEEDED(hr))
	{
		m_DevName = fcyStringHelper::WideCharToMultiByte(adapter_info.Description);
	}

	// create Direct3D11 basic components

	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL const target_levels[4] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	hr = gHR = D3D11CreateDevice(dxgi_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, creation_flags, target_levels, 4, D3D11_SDK_VERSION, &d3d11_device, &d3d11_level, &d3d11_devctx);
	if (FAILED(hr))
	{
		D3D_FEATURE_LEVEL const target_levels_downlevel[3] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		hr = gHR = D3D11CreateDevice(dxgi_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, creation_flags, target_levels_downlevel, 3, D3D11_SDK_VERSION, &d3d11_device, &d3d11_level, &d3d11_devctx);
	}
	if (FAILED(hr))
	{
		throw fcyWin32COMException("f2dRenderDevice11::f2dRenderDevice11", "D3D11CreateDevice failed.", hr);
	}

	// create swapchain

	if (!createSwapchain(nullptr))
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::createSwapchain failed.");
	}
}
f2dRenderDevice11::~f2dRenderDevice11()
{
	// 删除渲染器监听链
	for (auto& v : _setEventListeners)
	{
		// 报告可能的对象泄漏
		char tTextBuffer[256];
		sprintf_s(tTextBuffer, "Unrelease listener object at %p", v.listener);
#ifdef _DEBUG
		fcyDebug::Trace("[@f2dRenderDevice11::~f2dRenderDevice11] %s\n", tTextBuffer);
#endif
		m_pEngine->ThrowException(fcyException("f2dRenderDevice11::~f2dRenderDevice11", tTextBuffer));
	}
	_setEventListeners.clear();
}

void* f2dRenderDevice11::GetHandle() { return d3d11_device.Get(); }
fcStr f2dRenderDevice11::GetDeviceName() { return m_DevName.c_str(); }

// 设备丢失与恢复

int f2dRenderDevice11::sendDevLostMsg()
{
	int tRet = 0;

	// 发送丢失消息
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderDeviceLost();
		tRet += 1;
	}

	return tRet;
}
int f2dRenderDevice11::sendDevResetMsg()
{
	int tRet = 0;

	// 发送重置消息
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderDeviceReset();
		tRet += 1;
	}

	return tRet;
}
int f2dRenderDevice11::dispatchRenderSizeDependentResourcesCreate()
{
	int cnt = 0;
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesCreate();
		cnt += 1;
	}
	return cnt;
}
int f2dRenderDevice11::dispatchRenderSizeDependentResourcesDestroy()
{
	int cnt = 0;
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesDestroy();
		cnt += 1;
	}
	return cnt;
}

fResult f2dRenderDevice11::AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority)
{
	if (Listener == NULL)
		return FCYERR_INVAILDPARAM;

	EventListenerNode node;
	node.uuid = _iEventListenerUUID;
	node.priority = Priority;
	node.listener = Listener;
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
fResult f2dRenderDevice11::SyncDevice()
{
	// 需要重置设备
	if (m_bDevLost)
	{
		m_bDevLost = false;
		int tObjCount = sendDevResetMsg(); // 通知非托管组件恢复工作
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "device reset (%d object(s) reset)", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDevice11::SyncDevice", tBuffer));
		return FCYERR_OK;
	}
	// 小 Hack，在这里绑定交换链的 RenderTarget
	setupRenderAttachments();
	return FCYERR_OK;
}

// 创建资源

fResult f2dRenderDevice11::CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
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
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
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
		m_pEngine->ThrowException(e);
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
		m_pEngine->ThrowException(e);
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
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}
fResult f2dRenderDevice11::CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut)
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
		m_pEngine->ThrowException(e);
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

// 渲染附件

fResult f2dRenderDevice11::Clear(const fcyColor& BackBufferColor, fFloat ZValue)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		FLOAT clear_color[4] = {
			(float)BackBufferColor.r / 255.0f,
			(float)BackBufferColor.g / 255.0f,
			(float)BackBufferColor.b / 255.0f,
			(float)BackBufferColor.a / 255.0f,
		};
		d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, ZValue, 0);
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::Clear(const fcyColor& BackBufferColor, fFloat ZValue, fuInt StencilValue)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		FLOAT clear_color[4] = {
			(float)BackBufferColor.r / 255.0f,
			(float)BackBufferColor.g / 255.0f,
			(float)BackBufferColor.b / 255.0f,
			(float)BackBufferColor.a / 255.0f,
		};
		d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, ZValue, (UINT8)StencilValue);
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearColor(const fcyColor& BackBufferColor)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		FLOAT clear_color[4] = {
			(float)BackBufferColor.r / 255.0f,
			(float)BackBufferColor.g / 255.0f,
			(float)BackBufferColor.b / 255.0f,
			(float)BackBufferColor.a / 255.0f,
		};
		d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearZBuffer(fFloat Value)
{
	if (d3d11_devctx)
	{
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, Value, 0);
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearStencilBuffer(fuInt StencilValue)
{
	if (d3d11_devctx)
	{
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_STENCIL, 0.0f, (UINT8)StencilValue);
	}
	return FCYERR_OK;
}

f2dTexture2D* f2dRenderDevice11::GetRenderTarget()
{
	return *m_RenderTarget;
}
fResult f2dRenderDevice11::SetRenderTarget(f2dTexture2D* pTex)
{
	if (!pTex->IsRenderTarget())
		return FCYERR_INVAILDPARAM;

	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	if (*m_RenderTarget == pTex)
		return FCYERR_OK;

	m_RenderTarget = pTex; // 注意 pTex 可能是 NULL 代表需要重置为交换链的 RenderTarget

	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		d3d11_devctx->OMSetRenderTargets(1, &rtv, dsv);
	}

	return FCYERR_OK;
}
f2dDepthStencilSurface* f2dRenderDevice11::GetDepthStencilSurface()
{
	return *m_DepthStencil;
}
fResult f2dRenderDevice11::SetDepthStencilSurface(f2dDepthStencilSurface* pSurface)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	if (*m_DepthStencil == pSurface)
		return FCYERR_OK;

	m_DepthStencil = pSurface; // 注意 pSurface 可能是 NULL 代表需要重置为默认的 DepthStencil

	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		d3d11_devctx->OMSetRenderTargets(1, &rtv, dsv);
	}

	return FCYERR_OK;
}

// 状态设置

f2dGraphics* f2dRenderDevice11::QueryCurGraphics() { return m_pCurGraphics; }
fResult f2dRenderDevice11::SubmitCurGraphics(f2dGraphics* pGraph, bool bDirty)
{
	if (pGraph == NULL)
	{
		m_pCurGraphics = NULL;
		return FCYERR_OK;
	}
	else if (m_pCurGraphics)
	{
		if (m_pCurGraphics->IsInRender())
			return FCYERR_ILLEGAL;
	}

	if (!bDirty && pGraph == m_pCurGraphics)
		return FCYERR_OK;

	m_pCurGraphics = pGraph;

	return FCYERR_OK;
}
fcyRect f2dRenderDevice11::GetScissorRect()
{
	if (!d3d11_devctx)
	{
		return fcyRect();
	}
	UINT vpc = 0;
	D3D11_RECT vpv[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
	d3d11_devctx->RSGetScissorRects(&vpc, vpv);
	return fcyRect(
		(fFloat)vpv[0].left,
		(fFloat)vpv[0].top,
		(fFloat)vpv[0].right,
		(fFloat)vpv[0].bottom
	);
}
fResult f2dRenderDevice11::SetScissorRect(const fcyRect& pRect)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
	{
		m_pCurGraphics->Flush();
	}
	if (!d3d11_devctx)
	{
		return FCYERR_INTERNALERR;
	}
	D3D11_RECT d3d11_rc = {
		.left = (LONG)pRect.a.x,
		.top = (LONG)pRect.a.y,
		.right = (LONG)pRect.b.x,
		.bottom = (LONG)pRect.b.y,
	};
	d3d11_devctx->RSSetScissorRects(1, &d3d11_rc);
	return FCYERR_OK;
}
fcyRect f2dRenderDevice11::GetViewport()
{
	if (!d3d11_devctx)
	{
		return fcyRect();
	}
	UINT vpc = 0;
	D3D11_VIEWPORT vpv[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
	d3d11_devctx->RSGetViewports(&vpc, vpv);
	return fcyRect(
		vpv[0].TopLeftX,
		vpv[0].TopLeftY,
		vpv[0].TopLeftX + vpv[0].Width,
		vpv[0].TopLeftY + vpv[0].Height
	);
}
fResult f2dRenderDevice11::SetViewport(fcyRect vp)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
	{
		m_pCurGraphics->Flush();
	}
	if (!d3d11_devctx)
	{
		return FCYERR_INTERNALERR;
	}
	D3D11_VIEWPORT d3d11_vp = {
		.TopLeftX = vp.a.x,
		.TopLeftY = vp.a.y,
		.Width = vp.GetWidth(),
		.Height = vp.GetHeight(),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
	d3d11_devctx->RSSetViewports(1, &d3d11_vp);
	return FCYERR_OK;
}

// 交换链

void f2dRenderDevice11::destroySwapchain()
{
	destroyRenderAttachments();
	if (dxgi_swapchain)
	{
		BOOL bFullscreen = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
		if (SUCCEEDED(hr) || bFullscreen)
		{
			dxgi_swapchain->SetFullscreenState(FALSE, NULL); // 不关心结果，反正你给我离开独占全屏
		}
	}
	dxgi_swapchain.Reset();
}
bool f2dRenderDevice11::createSwapchain(f2dDisplayMode* pmode)
{
	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	dxgi_factory.As(&dxgi_factory2);
	Microsoft::WRL::ComPtr<ID3D11Device1> d3d11_device1;
	d3d11_device.As(&d3d11_device1);
	
	if (dxgi_factory2 && d3d11_device1)
	{
		// Windows 7 平台更新已安装，或者 Windows 8 及以上
		DXGI_SWAP_CHAIN_DESC1 desc1 = {
			.Width = swapchain_width,
			.Height = swapchain_height,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.Stereo = FALSE, // 永远用不上立体交换链
			.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1, .Quality = 0,},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.Scaling = DXGI_SCALING_STRETCH, // Windows 7 只支持这个
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD, // Windows 7 只支持这个
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE, // 永远用不上 A 通道
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC descf = {
			.RefreshRate = DXGI_RATIONAL{.Numerator = 0, .Denominator = 0,}, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Windowed = FALSE,
		};
		if (pmode)
		{
			desc1.Width = pmode->width;
			desc1.Height = pmode->height;
			descf.RefreshRate.Numerator = pmode->refresh_rate.numerator;
			descf.RefreshRate.Denominator = pmode->refresh_rate.denominator;
			descf.ScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)pmode->scanline_ordering;
			descf.Scaling = (DXGI_MODE_SCALING)pmode->scaling;
		}
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swapchain1;
		hr = gHR = dxgi_factory2->CreateSwapChainForHwnd(d3d11_device1.Get(), win32_window, &desc1, swapchain_windowed ? NULL : &descf, NULL, &dxgi_swapchain1);
		if (SUCCEEDED(hr))
		{
			hr = gHR = dxgi_swapchain1.As(&dxgi_swapchain);
		}
	}
	if (FAILED(hr))
	{
		// 回落到 Windows 7 的方式创建
		DXGI_MODE_DESC mode = {
			.Width = swapchain_width,
			.Height = swapchain_height,
			.RefreshRate = DXGI_RATIONAL{.Numerator = 0, .Denominator = 0,}, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
		};
		if (pmode)
		{
			mode.Width = pmode->width;
			mode.Height = pmode->height;
			mode.RefreshRate.Numerator = pmode->refresh_rate.numerator;
			mode.RefreshRate.Denominator = pmode->refresh_rate.denominator;
			mode.ScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)pmode->scanline_ordering;
			mode.Scaling = (DXGI_MODE_SCALING)pmode->scaling;
		}
		DXGI_SWAP_CHAIN_DESC desc = {
			.BufferDesc = mode,
			.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1, .Quality = 0,},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.OutputWindow = win32_window,
			.Windowed = swapchain_windowed,
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD, // Windows 7 只支持这个
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		hr = gHR = dxgi_factory->CreateSwapChain(d3d11_device.Get(), &desc, &dxgi_swapchain);
		if (FAILED(hr))
		{
			return false;
		}
	}

	if (!createRenderAttachments())
	{
		return false;
	}
	setupRenderAttachments();

	return true;
}
void f2dRenderDevice11::destroyRenderAttachments()
{
	if (d3d11_devctx)
	{
		d3d11_devctx->ClearState();
		d3d11_devctx->Flush();
	}
	d3d11_rendertarget.Reset();
	d3d11_depthstencil.Reset();
}
bool f2dRenderDevice11::createRenderAttachments()
{
	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgi_surface;
	hr = gHR = dxgi_swapchain->GetBuffer(0, IID_ID3D11Texture2D, &dxgi_surface);
	if (FAILED(hr))
	{
		return false;
	}
	hr = gHR = d3d11_device->CreateRenderTargetView(dxgi_surface.Get(), NULL, &d3d11_rendertarget);
	if (FAILED(hr))
	{
		return false;
	}
	
	D3D11_TEXTURE2D_DESC tex2ddef = {
		.Width = swapchain_width,
		.Height = swapchain_height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc = DXGI_SAMPLE_DESC{.Count = 1, .Quality = 0,},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d;
	hr = gHR = d3d11_device->CreateTexture2D(&tex2ddef, NULL, &d3d11_texture2d);
	if (FAILED(hr))
	{
		return false;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdef = {
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = D3D11_TEX2D_DSV{.MipSlice = 0,},
	};
	hr = gHR = d3d11_device->CreateDepthStencilView(d3d11_texture2d.Get(), &dsvdef, &d3d11_depthstencil);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}
void f2dRenderDevice11::setupRenderAttachments()
{
	if (d3d11_devctx && d3d11_rendertarget && d3d11_depthstencil)
	{
		ID3D11RenderTargetView* rtv = d3d11_rendertarget.Get();
		d3d11_devctx->OMSetRenderTargets(1, &rtv, d3d11_depthstencil.Get());
	}
}

fuInt f2dRenderDevice11::GetSupportedDisplayModeCount(fBool refresh)
{
	if (display_modes.empty() || refresh)
	{
		display_modes.clear();
		HRESULT hr = 0;

		if (!dxgi_swapchain) return 0;
		
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
		if (FAILED(hr)) return 0;

		UINT mode_count = 0;
		hr = gHR = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, NULL);
		if (FAILED(hr)) return 0;
		std::vector<DXGI_MODE_DESC> modes(mode_count);
		hr = gHR = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, modes.data());
		if (FAILED(hr)) return 0;

		display_modes.resize(mode_count);
		for (UINT i = 0; i < mode_count; i += 1)
		{
			display_modes[i].width = modes[i].Width;
			display_modes[i].height = modes[i].Height;
			display_modes[i].refresh_rate.numerator = modes[i].RefreshRate.Numerator;
			display_modes[i].refresh_rate.denominator = modes[i].RefreshRate.Denominator;
			display_modes[i].format = (fuInt)modes[i].Format;
			display_modes[i].scanline_ordering = (fuInt)modes[i].ScanlineOrdering;
			display_modes[i].scaling = (fuInt)modes[i].Scaling;
		}
	}
	return (fuInt)display_modes.size();
}
f2dDisplayMode f2dRenderDevice11::GetSupportedDisplayMode(fuInt Index)
{
	assert(Index < display_modes.size());
	return display_modes[Index];
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel)
{
	if (Width < 1 || Height < 1)
		return FCYERR_INVAILDPARAM;
	swapchain_width = Width;
	swapchain_height = Height;
	swapchain_windowed = true;
	swapchain_vsync = VSync;
	swapchain_flip = FlipModel;
	dispatchRenderSizeDependentResourcesDestroy();
	destroySwapchain();
	if (!createSwapchain(nullptr))
	{
		return FCYERR_INTERNALERR;
	}
	dispatchRenderSizeDependentResourcesCreate();
	return FCYERR_OK;
}
fResult f2dRenderDevice11::SetDisplayMode(f2dDisplayMode mode, fBool VSync)
{
	if (mode.width < 1 || mode.height < 1)
		return FCYERR_INVAILDPARAM;
	swapchain_width = mode.width;
	swapchain_height = mode.height;
	swapchain_windowed = false;
	swapchain_vsync = VSync;
	swapchain_flip = false;
	dispatchRenderSizeDependentResourcesDestroy();
	destroySwapchain();
	if (!createSwapchain(&mode))
	{
		return FCYERR_INTERNALERR;
	}
	dispatchRenderSizeDependentResourcesCreate();
	HRESULT hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
	if (FAILED(hr))
	{
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}
fuInt f2dRenderDevice11::GetBufferWidth() { return swapchain_width; }
fuInt f2dRenderDevice11::GetBufferHeight() { return swapchain_height; }
fBool f2dRenderDevice11::IsWindowed() { return swapchain_windowed; }
fResult f2dRenderDevice11::Present()
{
	if (!dxgi_swapchain)
	{
		return FCYERR_INTERNALERR;
	}

	setupRenderAttachments(); // 重新绑定回原来的
	m_RenderTarget = nullptr;
	m_DepthStencil = nullptr;

	HRESULT hr = gHR = dxgi_swapchain->Present(swapchain_vsync ? 1 : 0, 0);
	if (hr != S_OK && hr != DXGI_STATUS_OCCLUDED && hr != DXGI_STATUS_MODE_CHANGED && hr != DXGI_STATUS_MODE_CHANGE_IN_PROGRESS)
	{
		// 设备丢失，广播设备丢失事件
		m_bDevLost = true; // 标记为设备丢失状态
		int tObjCount = sendDevLostMsg();
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "device lost (%d object(s) lost)", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDevice11::Present", tBuffer));
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}

// 纹理读写

fResult f2dRenderDevice11::SaveScreen(f2dStream* pStream) { return FCYERR_NOTSUPPORT; }
fResult f2dRenderDevice11::SaveTexture(f2dStream* pStream, f2dTexture2D* pTex) { return FCYERR_NOTSUPPORT; }
fResult f2dRenderDevice11::SaveScreen(fcStrW path)
{
	if (!path)
		return FCYERR_INVAILDPARAM;
	if (!d3d11_devctx || !d3d11_rendertarget)
		return FCYERR_ILLEGAL;

	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<ID3D11Resource> d3d11_resource;
	d3d11_rendertarget->GetResource(&d3d11_resource);
	hr = gHR = DirectX::SaveWICTextureToFile(d3d11_devctx.Get(), d3d11_resource.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if (FAILED(hr))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDevice11::SaveScreen", "DirectX::SaveWICTextureToFile failed", hr));
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
		f2dTexture2D11* pRTex = dynamic_cast<f2dTexture2D11*>(pTex);
		d3d11_resource = pRTex->GetResource();
	}
	else
	{
		f2dRenderTarget11* pRTex = dynamic_cast<f2dRenderTarget11*>(pTex);
		d3d11_resource = pRTex->GetResource();
	}
	hr = gHR = DirectX::SaveWICTextureToFile(d3d11_devctx.Get(), d3d11_resource.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if (FAILED(hr))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDevice11::SaveScreen", "DirectX::SaveWICTextureToFile failed", hr));
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

// 废弃的方法集合，大部分是固定管线遗毒，部分是完全没做好的功能，或者不适应新的图形API

fResult f2dRenderDevice11::SubmitWorldMat(const fcyMatrix4& Mat)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitLookatMat(const fcyMatrix4& Mat)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitProjMat(const fcyMatrix4& Mat)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitBlendState(const f2dBlendState& State)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitVD(IDirect3DVertexDeclaration9* pVD)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitTextureBlendOP_Color(D3DTEXTUREOP ColorOP)
{
	return FCYERR_NOTIMPL;
}
F2DAALEVEL f2dRenderDevice11::GetAALevel()
{
	return F2DAALEVEL_NONE;
}
fBool f2dRenderDevice11::CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed)
{
	return false;
}
fBool f2dRenderDevice11::IsZBufferEnabled()
{
	return false;
}
fResult f2dRenderDevice11::SetZBufferEnable(fBool v)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::UpdateScreenToWindow(fcyColor KeyColor, fByte Alpha)
{
	// 应该考虑用 DirectComposition 代替
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetTextureFilter(F2DTEXFILTERTYPE filter)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut)
{
	// 设计的很固定管线，所以抱歉了
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut)
{
	// 就支持个破屏幕后处理，也配叫 Graphics3D ？你配吗？配几把？
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut)
{
	// 时代眼泪 Effect 框架
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut)
{
	// 模型功能也没有完成
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel)
{
	if (Windowed)
	{
		return SetDisplayMode(Width, Height, VSync, FlipModel);
	}
	else
	{
		if (d3d11_device && dxgi_swapchain)
		{
			HRESULT hr = 0;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
			if (SUCCEEDED(hr))
			{
				DXGI_MODE_DESC target_mode = {
					.Width = Width,
					.Height = Height,
					.RefreshRate = DXGI_RATIONAL{.Numerator = 60,.Denominator = 1},
					.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
					.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
					.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
				};
				DXGI_MODE_DESC mode = {};
				hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &mode, d3d11_device.Get());
				if (SUCCEEDED(hr))
				{
					f2dDisplayMode f2dmode = {
						.width = mode.Width,
						.height = mode.Height,
						.refresh_rate = f2dRational{.numerator = mode.RefreshRate.Numerator, .denominator = mode.RefreshRate.Denominator},
						.format = (fuInt)mode.Format,
						.scanline_ordering = (fuInt)mode.ScanlineOrdering,
						.scaling = (fuInt)mode.Scaling,
					};
					return SetDisplayMode(f2dmode, VSync);
				}
			}
		}
		return FCYERR_INVAILDPARAM;
	}
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRate, fBool Windowed, fBool VSync, fBool FlipModel)
{
	if (Windowed)
	{
		return SetDisplayMode(Width, Height, VSync, FlipModel);
	}
	else
	{
		if (d3d11_device && dxgi_swapchain)
		{
			HRESULT hr = 0;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
			if (SUCCEEDED(hr))
			{
				DXGI_MODE_DESC target_mode = {
					.Width = Width,
					.Height = Height,
					.RefreshRate = DXGI_RATIONAL{.Numerator = RefreshRate,.Denominator = 1},
					.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
					.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE,
					.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
				};
				DXGI_MODE_DESC mode = {};
				hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &mode, d3d11_device.Get());
				if (SUCCEEDED(hr))
				{
					f2dDisplayMode f2dmode = {
						.width = mode.Width,
						.height = mode.Height,
						.refresh_rate = f2dRational{.numerator = mode.RefreshRate.Numerator, .denominator = mode.RefreshRate.Denominator},
						.format = (fuInt)mode.Format,
						.scanline_ordering = (fuInt)mode.ScanlineOrdering,
						.scaling = (fuInt)mode.Scaling,
					};
					return SetDisplayMode(f2dmode, VSync);
				}
			}
		}
		return FCYERR_INVAILDPARAM;
	}
}
