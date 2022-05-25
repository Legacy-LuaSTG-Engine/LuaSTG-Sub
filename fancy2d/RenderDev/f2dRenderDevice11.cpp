#include "RenderDev/f2dRenderDevice11.h"

#include <fcyOS/fcyDebug.h>
#include "RenderDev/f2dTexture11.h"
#include "Engine/f2dEngineImpl.h"

#include "utility/encoding.hpp"
#include "platform/WindowsVersion.hpp"
#include "Core/Graphics/Device_D3D11.hpp"

static std::string bytes_count_to_string(DWORDLONG size)
{
	int count = 0;
	char buffer[64] = {};
	if (size < 1024llu) // B
	{
		count = std::snprintf(buffer, 64, "%u B", (unsigned int)size);
	}
	else if (size < (1024llu * 1024llu)) // KB
	{
		count = std::snprintf(buffer, 64, "%.2f KB", (double)size / 1024.0);
	}
	else if (size < (1024llu * 1024llu * 1024llu)) // MB
	{
		count = std::snprintf(buffer, 64, "%.2f MB", (double)size / 1048576.0);
	}
	else // GB
	{
		count = std::snprintf(buffer, 64, "%.2f GB", (double)size / 1073741824.0);
	}
	return std::string(buffer, count);
}
static void get_system_memory_status()
{
	MEMORYSTATUSEX info = { sizeof(MEMORYSTATUSEX) };
	if (GlobalMemoryStatusEx(&info))
	{
		spdlog::info("[fancy2d] 系统内存使用情况：\n"
			"    使用百分比：{}%\n"
			"    总物理内存：{}\n"
			"    剩余物理内存：{}\n"
			"    当前进程可提交内存限制：{}\n"
			"    当前进程剩余的可提交内存：{}\n"
			"    当前进程用户模式内存空间限制*1：{}\n"
			"    当前进程剩余的用户模式内存空间：{}\n"
			"        *1 此项反映此程序实际上能用的最大内存，在 32 位应用程序上此项一般为 2 GB，修改 Windows 操作系统注册表后可能为 1 到 3 GB"
			, info.dwMemoryLoad
			, bytes_count_to_string(info.ullTotalPhys)
			, bytes_count_to_string(info.ullAvailPhys)
			, bytes_count_to_string(info.ullTotalPageFile)
			, bytes_count_to_string(info.ullAvailPageFile)
			, bytes_count_to_string(info.ullTotalVirtual)
			, bytes_count_to_string(info.ullAvailVirtual)
		);
	}
	else
	{
		spdlog::error("[fancy2d] 无法获取系统内存使用情况");
	}
}

// 类主体

f2dRenderDevice11::f2dRenderDevice11(f2dEngineImpl* pEngine, f2dEngineRenderWindowParam* RenderWindowParam)
	: m_pEngine(pEngine)
	, m_CreateThreadID(GetCurrentThreadId())
	, m_PreferDevName(RenderWindowParam->gpu ? RenderWindowParam->gpu : L"")
	, swapchain_width(RenderWindowParam->mode.width)
	, swapchain_height(RenderWindowParam->mode.height)
	, swapchain_windowed(true) // 必须以窗口模式启动
	, swapchain_vsync(RenderWindowParam->vsync)
{
	m_hWnd = (HWND)pEngine->GetMainWindow()->GetHandle();
	win32_window = m_hWnd;

	spdlog::info("[fancy2d] 操作系统版本：{}", platform::WindowsVersion::GetName());
	get_system_memory_status();

	// 设备族

	if (!LuaSTG::Core::Graphics::IDevice::create(utility::encoding::to_utf8(m_PreferDevName), ~m_pGraphicsDevice))
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "LuaSTG::Core::Graphics::IDevice::create failed");
	}

	auto* p_dev = dynamic_cast<LuaSTG::Core::Graphics::Device_D3D11*>(*m_pGraphicsDevice);

	m_DevName = p_dev->GetAdapterName();
	m_DevList = p_dev->GetAdapterNameArray();

	dxgi_factory = p_dev->GetDXGIFactory1();
	dxgi_adapter = p_dev->GetDXGIAdapter1();

	d3d11_device = p_dev->GetD3D11Device();
	d3d11_devctx = p_dev->GetD3D11DeviceContext();
	d3d11_level = p_dev->GetD3DFeatureLevel();

	dxgi_support_flipmodel = p_dev->IsFlipSequentialSupport();
	dxgi_support_flipmodel2 = p_dev->IsFlipDiscardSupport();
	dxgi_support_lowlatency = p_dev->IsFrameLatencySupport();
	dxgi_support_tearing = p_dev->IsTearingSupport();

	// 交换链

	if (!createSwapchain(nullptr))
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::createSwapchain failed.");
	}
	GetSupportedDisplayModeCount(true);

	m_pEngine->GetMainWindow()->SetGraphicListener(this);

	tracy::xTracyD3D11Context(d3d11_device.Get(), d3d11_devctx.Get());

	// 如果需要，试着进入全屏模式

	if (!RenderWindowParam->windowed)
	{
		auto& mode = RenderWindowParam->mode;
		SetDisplayMode(
			mode.width, mode.height,
			mode.refresh_rate.numerator, mode.refresh_rate.denominator,
			RenderWindowParam->windowed, RenderWindowParam->vsync, false);
	}
}
f2dRenderDevice11::~f2dRenderDevice11()
{
	tracy::xTracyD3D11Destroy();

	if (m_pEngine->GetMainWindow())
		m_pEngine->GetMainWindow()->SetGraphicListener(nullptr);

	beforeDestroyDevice(); // 一定要先退出独占全屏

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
fuInt f2dRenderDevice11::GetSupportedDeviceCount() { return m_DevList.size(); }
fcStr f2dRenderDevice11::GetSupportedDeviceName(fuInt Index) { return m_DevList[Index].c_str(); }
f2dAdapterMemoryUsageStatistics f2dRenderDevice11::GetAdapterMemoryUsageStatistics()
{
	f2dAdapterMemoryUsageStatistics data = {};
	Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
	if (bHR = dxgi_adapter.As(&adapter))
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
		if (bHR = gHR = adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info))
		{
			data.local.budget = info.Budget;
			data.local.current_usage = info.CurrentUsage;
			data.local.available_for_reservation = info.AvailableForReservation;
			data.local.current_reservation = info.CurrentReservation;
		}
		if (bHR = gHR = adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info))
		{
			data.non_local.budget = info.Budget;
			data.non_local.current_usage = info.CurrentUsage;
			data.non_local.available_for_reservation = info.AvailableForReservation;
			data.non_local.current_reservation = info.CurrentReservation;
		}
	}
	return data;
}

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
	// 要退出全屏
	if (swapchain_want_exit_fullscreen)
	{
		swapchain_want_exit_fullscreen = false;
		if (!swapchain_windowed && dxgi_swapchain)
		{
			BOOL bFSC = FALSE;
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
			HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFSC, &dxgi_output);
			if (bFSC)
			{
				spdlog::info("[fancy2d] 尝试退出独占全屏");
				hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
				if (FAILED(hr))
				{
					spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
				}
			}
		}
		m_pEngine->GetMainWindow()->SetTopMost(false);
	}
	else
	{
		// 试着重新进入全屏
		if (swapchain_want_enter_fullscreen)
		{
			swapchain_want_enter_fullscreen = false;
			if (!swapchain_windowed && dxgi_swapchain)
			{
				BOOL bFSC = FALSE;
				Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
				HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFSC, &dxgi_output);
				if (FAILED(hr) || !bFSC)
				{
					spdlog::info("[fancy2d] 尝试切换到独占全屏");
					hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
					if (FAILED(hr))
					{
						spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #TRUE 调用失败，无法进入独占全屏");
					}
				}
			}
		}
	}
	// 需要重新调整交换链大小
	if (swapchain_want_resize)
	{
		swapchain_want_resize = false;
		destroyRenderAttachments();
		if (dxgi_swapchain)
		{
			HRESULT hr = gHR = dxgi_swapchain->ResizeBuffers(swapchain_resize_data.BufferCount, swapchain_width, swapchain_height, swapchain_resize_data.Format, swapchain_resize_data.Flags);
			if (FAILED(hr))
			{
				return FCYERR_INTERNALERR;
			}
		}
		if (!createRenderAttachments())
		{
			return FCYERR_INTERNALERR;
		}
	}
	// 小 Hack，在这里绑定交换链的 RenderTarget
	setupRenderAttachments();
	return FCYERR_OK;
}
void f2dRenderDevice11::OnLostFocus()
{
	swapchain_want_exit_fullscreen = true;
}
void f2dRenderDevice11::OnGetFocus()
{
	swapchain_want_enter_fullscreen = true;
}
void f2dRenderDevice11::OnSize(fuInt ClientWidth, fuInt ClientHeight)
{
	std::ignore = ClientWidth;
	std::ignore = ClientHeight;
	swapchain_want_resize = true;
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
		m_pEngine->ThrowException(e);
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
		if (rtv)
		{
			FLOAT clear_color[4] = {
				(float)BackBufferColor.r / 255.0f,
				(float)BackBufferColor.g / 255.0f,
				(float)BackBufferColor.b / 255.0f,
				(float)BackBufferColor.a / 255.0f,
			};
			d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		}
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, ZValue, 0);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::Clear(const fcyColor& BackBufferColor, fFloat ZValue, fuInt StencilValue)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (rtv)
		{
			FLOAT clear_color[4] = {
				(float)BackBufferColor.r / 255.0f,
				(float)BackBufferColor.g / 255.0f,
				(float)BackBufferColor.b / 255.0f,
				(float)BackBufferColor.a / 255.0f,
			};
			d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		}
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, ZValue, (UINT8)StencilValue);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearColor(const fcyColor& BackBufferColor)
{
	if (d3d11_devctx)
	{
		ID3D11RenderTargetView* rtv = *m_RenderTarget ? ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView() : d3d11_rendertarget.Get();
		if (rtv)
		{
			FLOAT clear_color[4] = {
				(float)BackBufferColor.r / 255.0f,
				(float)BackBufferColor.g / 255.0f,
				(float)BackBufferColor.b / 255.0f,
				(float)BackBufferColor.a / 255.0f,
			};
			d3d11_devctx->ClearRenderTargetView(rtv, clear_color);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearZBuffer(fFloat Value)
{
	if (d3d11_devctx)
	{
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, Value, 0);
		}
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::ClearStencilBuffer(fuInt StencilValue)
{
	if (d3d11_devctx)
	{
		ID3D11DepthStencilView* dsv = *m_DepthStencil ? ((f2dDepthStencil11*)*m_DepthStencil)->GetView() : d3d11_depthstencil.Get();
		if (dsv)
		{
			d3d11_devctx->ClearDepthStencilView(dsv, D3D11_CLEAR_STENCIL, 0.0f, (UINT8)StencilValue);
		}
	}
	return FCYERR_OK;
}

f2dTexture2D* f2dRenderDevice11::GetRenderTarget()
{
	return *m_RenderTarget;
}
fResult f2dRenderDevice11::SetRenderTarget(f2dTexture2D* pTex)
{
	if (pTex && !pTex->IsRenderTarget())
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

void f2dRenderDevice11::beforeDestroyDevice()
{
	if (d3d11_devctx)
	{
		d3d11_devctx->ClearState();
		d3d11_devctx->Flush();
	}
	if (dxgi_swapchain)
	{
		BOOL bFullscreen = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
		if (bFullscreen)
		{
			spdlog::info("[fancy2d] 尝试退出独占全屏");
			hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
			}
		}
	}
}
void f2dRenderDevice11::destroySwapchain()
{
	destroyRenderAttachments();
	if (dxgi_swapchain)
	{
		BOOL bFullscreen = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		HRESULT hr = gHR = dxgi_swapchain->GetFullscreenState(&bFullscreen, &dxgi_output);
		if (bFullscreen)
		{
			spdlog::info("[fancy2d] 尝试退出独占全屏");
			hr = gHR = dxgi_swapchain->SetFullscreenState(FALSE, NULL);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #FALSE 调用失败");
			}
		}
	}
	dxgi_swapchain_event.Close();
	dxgi_swapchain.Reset();
}
bool f2dRenderDevice11::createSwapchain(f2dDisplayMode* pmode)
{
	HRESULT hr = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgi_factory2;
	dxgi_factory.As(&dxgi_factory2);
	Microsoft::WRL::ComPtr<ID3D11Device1> d3d11_device1;
	d3d11_device.As(&d3d11_device1);
	
	spdlog::info("[fancy2d] 开始创建 SwapChain（交换链）组件");
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
			.BufferCount = 1,
			.Scaling = DXGI_SCALING_STRETCH, // Windows 7 只支持这个
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD, // Windows 7 只支持这个
			.AlphaMode = DXGI_ALPHA_MODE_IGNORE, // 永远用不上 A 通道
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC descf = {
			.RefreshRate = DXGI_RATIONAL{.Numerator = 0, .Denominator = 0,}, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Scaling = DXGI_MODE_SCALING_UNSPECIFIED, // 警告：这些数值全™是我杜撰的，要是程序崩溃了不关我的事情
			.Windowed = TRUE,
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
		if (swapchain_windowed && swapchain_flip)
		{
			// 只有在窗口模式下才允许开这个功能
			if (dxgi_support_flipmodel2 && dxgi_support_lowlatency && dxgi_support_tearing)
			{
				desc1.BufferCount = 2;
				desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				swapchain_resize_data.FrameLatencyWaitableObject = TRUE;
				swapchain_resize_data.AllowTearing = TRUE;
			}
			else if (dxgi_support_flipmodel2 && dxgi_support_lowlatency)
			{
				desc1.BufferCount = 2;
				desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				swapchain_resize_data.FrameLatencyWaitableObject = TRUE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
			else if (dxgi_support_flipmodel && dxgi_support_lowlatency)
			{
				desc1.BufferCount = 2;
				desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				desc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				swapchain_resize_data.FrameLatencyWaitableObject = TRUE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
			else if (dxgi_support_flipmodel)
			{
				desc1.BufferCount = 2;
				desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				swapchain_resize_data.FrameLatencyWaitableObject = FALSE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
			else
			{
				swapchain_resize_data.FrameLatencyWaitableObject = FALSE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
		}
		else
		{
			swapchain_resize_data.FrameLatencyWaitableObject = FALSE;
			swapchain_resize_data.AllowTearing = FALSE;
		}
		swapchain_resize_data.BufferCount = desc1.BufferCount;
		swapchain_resize_data.Format = desc1.Format;
		swapchain_resize_data.Flags = desc1.Flags;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgi_swapchain1;
		hr = gHR = dxgi_factory2->CreateSwapChainForHwnd(d3d11_device1.Get(), win32_window, &desc1, swapchain_windowed ? NULL : &descf, NULL, &dxgi_swapchain1);
		if (SUCCEEDED(hr))
		{
			hr = gHR = dxgi_swapchain1.As(&dxgi_swapchain);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGISwapChain1::QueryInterface -> #IDXGISwapChain 调用失败，创建 SwapChain（交换链）组件失败");
			}
		}
		else
		{
			spdlog::error("[fancy2d] IDXGIFactory2::CreateSwapChainForHwnd 调用失败，创建 SwapChain（交换链）组件失败");
		}
	}
	if (!dxgi_swapchain)
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
			.BufferCount = 1,
			.OutputWindow = win32_window,
			.Windowed = TRUE,
			.SwapEffect = DXGI_SWAP_EFFECT_DISCARD, // Windows 7 只支持这个
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		if (swapchain_windowed && swapchain_flip)
		{
			// 只有在窗口模式下才允许开这个功能
			if (dxgi_support_flipmodel2 && dxgi_support_lowlatency && dxgi_support_tearing)
			{
				desc.BufferCount = 2;
				desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				desc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				swapchain_resize_data.FrameLatencyWaitableObject = TRUE;
				swapchain_resize_data.AllowTearing = TRUE;
			}
			else if (dxgi_support_flipmodel2 && dxgi_support_lowlatency)
			{
				desc.BufferCount = 2;
				desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				desc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				swapchain_resize_data.FrameLatencyWaitableObject = TRUE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
			else if (dxgi_support_flipmodel && dxgi_support_lowlatency)
			{
				desc.BufferCount = 2;
				desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				desc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
				swapchain_resize_data.FrameLatencyWaitableObject = TRUE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
			else if (dxgi_support_flipmodel)
			{
				desc.BufferCount = 2;
				desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
				swapchain_resize_data.FrameLatencyWaitableObject = FALSE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
			else
			{
				swapchain_resize_data.FrameLatencyWaitableObject = FALSE;
				swapchain_resize_data.AllowTearing = FALSE;
			}
		}
		else
		{
			swapchain_resize_data.FrameLatencyWaitableObject = FALSE;
			swapchain_resize_data.AllowTearing = FALSE;
		}
		swapchain_resize_data.BufferCount = desc.BufferCount;
		swapchain_resize_data.Format = mode.Format;
		swapchain_resize_data.Flags = desc.Flags;
		hr = gHR = dxgi_factory->CreateSwapChain(d3d11_device.Get(), &desc, &dxgi_swapchain);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIFactory1::CreateSwapChain 调用失败，创建 SwapChain（交换链）组件失败");
			return false;
		}
	}
	if (dxgi_swapchain)
	{
		spdlog::info("[fancy2d] 已创建 SwapChain（交换链）组件");
	}
	
	hr = gHR = dxgi_factory->MakeWindowAssociation(win32_window, DXGI_MWA_NO_ALT_ENTER); // 别他妈乱切换了
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IDXGIFactory1::MakeWindowAssociation 调用失败，无法关闭 DXGI 自带的 ALT+ENTER 切换全屏功能（该功能会导致画面显示异常）");
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGISwapChain2> dxgi_swapchain2;
	hr = gHR = dxgi_swapchain.As(&dxgi_swapchain2);
	if (swapchain_resize_data.FrameLatencyWaitableObject && SUCCEEDED(hr))
	{
		hr = gHR = dxgi_swapchain2->SetMaximumFrameLatency(1);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGISwapChain2::SetMaximumFrameLatency -> #1 调用失败");
		}
		dxgi_swapchain_event.Attach(dxgi_swapchain2->GetFrameLatencyWaitableObject());
		if (!dxgi_swapchain_event.IsValid())
		{
			spdlog::error("[fancy2d] IDXGISwapChain2::GetFrameLatencyWaitableObject 调用失败");
		}
	}
	else
	{
		Microsoft::WRL::ComPtr<IDXGIDevice1> dxgi_device1;
		hr = gHR = d3d11_device.As(&dxgi_device1);
		if (SUCCEEDED(hr))
		{
			hr = gHR = dxgi_device1->SetMaximumFrameLatency(1);
			if (FAILED(hr))
			{
				spdlog::error("[fancy2d] IDXGIDevice1::SetMaximumFrameLatency -> #1 调用失败");
			}
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
	if (!dxgi_swapchain)
	{
		return false;
	}

	spdlog::info("[fancy2d] 开始创建 RenderAttachment（渲染附件）");

	HRESULT hr = 0;

	spdlog::info("[fancy2d] 开始创建 RenderTarget（渲染目标）");
	Microsoft::WRL::ComPtr<ID3D11Texture2D> dxgi_surface;
	hr = gHR = dxgi_swapchain->GetBuffer(0, IID_ID3D11Texture2D, &dxgi_surface);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IDXGISwapChain::GetBuffer -> #0 调用失败，无法获得 BackBuffer（后备缓冲区）");
		return false;
	}
	hr = gHR = d3d11_device->CreateRenderTargetView(dxgi_surface.Get(), NULL, &d3d11_rendertarget);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] ID3D11Device::CreateRenderTargetView 调用失败");
		return false;
	}
	spdlog::info("[fancy2d] 已创建 RenderTarget（渲染目标）");

	spdlog::info("[fancy2d] 开始创建 DepthStencilBuffer（深度&模板缓冲区）");
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
		spdlog::error("[fancy2d] ID3D11Device::CreateTexture2D 调用失败");
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
		spdlog::error("[fancy2d] ID3D11Device::CreateDepthStencilView 调用失败");
		return false;
	}
	spdlog::info("[fancy2d] 已创建 DepthStencilBuffer（深度&模板缓冲区）");

	spdlog::info("[fancy2d] 已创建 RenderAttachment（渲染附件）");

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

		spdlog::info("[fancy2d] 开始枚举支持的显示模式");

		if (!dxgi_swapchain)
		{
			spdlog::error("[fancy2d] 无法获得 Output（显示输出）设备");
			return 0;
		}
		
		Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
		hr = gHR = dxgi_swapchain->GetContainingOutput(&dxgi_output);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGISwapChain::GetContainingOutput 调用失败，无法获得 Output（显示输出）设备");
			return 0;
		}

		UINT mode_count = 0;
		hr = gHR = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, NULL);
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIOutput::GetDisplayModeList 调用失败");
			return 0;
		}
		std::vector<DXGI_MODE_DESC> modes(mode_count);
		hr = gHR = dxgi_output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, modes.data());
		if (FAILED(hr))
		{
			spdlog::error("[fancy2d] IDXGIOutput::GetDisplayModeList 调用失败");
			return 0;
		}

		display_modes.reserve(mode_count);
		for (UINT i = 0; i < mode_count; i += 1)
		{
			if (
				((double)modes[i].RefreshRate.Numerator / (double)modes[i].RefreshRate.Denominator) >= 58.5
				&& (modes[i].Width >= 640 || modes[i].Height >= 360)
				&& (modes[i].ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED || modes[i].ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE)
				&& modes[i].Scaling == DXGI_MODE_SCALING_UNSPECIFIED)
			{
				
				display_modes.emplace_back(f2dDisplayMode{
					.width = modes[i].Width,
					.height = modes[i].Height,
					.refresh_rate = f2dRational{
						.numerator = modes[i].RefreshRate.Numerator,
						.denominator = modes[i].RefreshRate.Denominator,
					},
					.format = (fuInt)modes[i].Format,
					.scanline_ordering = (fuInt)modes[i].ScanlineOrdering,
					.scaling = (fuInt)modes[i].Scaling,
				});
			}
		}
		if (!display_modes.empty())
		{
			spdlog::info("[fancy2d] 共找到 {} 个支持的显示模式：", display_modes.size());
			for (size_t i = 0; i < display_modes.size(); i += 1)
			{
				spdlog::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz"
					, i
					, display_modes[i].width, display_modes[i].height
					, (double)display_modes[i].refresh_rate.numerator / (double)display_modes[i].refresh_rate.denominator
				);
			}
		}
		else
		{
			for (UINT i = 0; i < mode_count; i += 1)
			{
				display_modes.emplace_back(f2dDisplayMode{
						.width = modes[i].Width,
						.height = modes[i].Height,
						.refresh_rate = f2dRational{
							.numerator = modes[i].RefreshRate.Numerator,
							.denominator = modes[i].RefreshRate.Denominator,
						},
						.format = (fuInt)modes[i].Format,
						.scanline_ordering = (fuInt)modes[i].ScanlineOrdering,
						.scaling = (fuInt)modes[i].Scaling,
					});
			}
			if (!display_modes.empty())
			{
				spdlog::warn("[fancy2d] 找不到支持的显示模式，开始查找兼容的显示模式");
				spdlog::info("[fancy2d] 共找到 {} 个兼容的显示模式：", display_modes.size());
				for (size_t i = 0; i < display_modes.size(); i += 1)
				{
					spdlog::info("{: >4d}: ({: >5d} x {: >5d}) {:.2f}Hz"
						, i
						, display_modes[i].width, display_modes[i].height
						, (double)display_modes[i].refresh_rate.numerator / (double)display_modes[i].refresh_rate.denominator
					);
				}
			}
			else
			{
				spdlog::error("[fancy2d] 枚举支持的显示模式失败，没有可用的显示模式");
				display_modes.clear();
				return 0;
			}
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
	((f2dWindowImpl*)m_pEngine->GetMainWindow())->MoveMouseToRightBottom();
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
	swapchain_flip = false; // 独占全屏永远不能开这个功能
	dispatchRenderSizeDependentResourcesDestroy();
	destroySwapchain();
	if (!createSwapchain(&mode))
	{
		return FCYERR_INTERNALERR;
	}
	dispatchRenderSizeDependentResourcesCreate();
	// 进入全屏
	spdlog::info("[fancy2d] 尝试切换到独占全屏");
	HRESULT hr = gHR = dxgi_swapchain->SetFullscreenState(TRUE, NULL);
	if (FAILED(hr))
	{
		spdlog::error("[fancy2d] IDXGISwapChain::SetFullscreenState -> #TRUE 调用失败，无法进入独占全屏");
		return FCYERR_INTERNALERR;
	}
	((f2dWindowImpl*)m_pEngine->GetMainWindow())->MoveMouseToRightBottom();
	return FCYERR_OK;
}
fuInt f2dRenderDevice11::GetBufferWidth() { return swapchain_width; }
fuInt f2dRenderDevice11::GetBufferHeight() { return swapchain_height; }
fBool f2dRenderDevice11::IsWindowed() { return swapchain_windowed; }
fResult f2dRenderDevice11::WaitDevice()
{
	if (swapchain_resize_data.FrameLatencyWaitableObject && dxgi_swapchain_event.IsValid())
	{
		WaitForSingleObject(dxgi_swapchain_event.Get(), 1000);
	}
	return FCYERR_OK;
}
fResult f2dRenderDevice11::Present()
{
	if (!dxgi_swapchain)
	{
		return FCYERR_INTERNALERR;
	}

	setupRenderAttachments(); // 重新绑定回原来的
	m_RenderTarget = nullptr;
	m_DepthStencil = nullptr;

	UINT interval = swapchain_vsync ? 1 : 0;
	UINT flags = (swapchain_resize_data.AllowTearing && (!swapchain_vsync)) ? DXGI_PRESENT_ALLOW_TEARING : 0; // 这个功能只有窗口下才能开
	HRESULT hr = gHR = dxgi_swapchain->Present(interval, flags);
	if (hr != S_OK && hr != DXGI_STATUS_OCCLUDED && hr != DXGI_STATUS_MODE_CHANGED && hr != DXGI_STATUS_MODE_CHANGE_IN_PROGRESS)
	{
		spdlog::critical("[fancy2d] IDXGISwapChain::Present 调用失败，注意：设备已丢失，接下来的渲染将无法正常进行");
		spdlog::info("[fancy2d] 可能导致设备丢失的情况：\n"
			"    1、其他应用程序独占图形设备\n"
			"    2、图形设备驱动程序因驱动更新而重置\n"
			"    3、图形设备驱动程序崩溃\n"
			"    4、图形设备因运行异常，或电脑休眠/睡眠，而停止工作\n"
			"    5、图形设备已被移除*1\n"
			"    6、其他意外情况*2\n"
			"        *1 注意，某些电脑的显卡是热插拔的，比如某些平板、笔记本二合一电脑，独立显卡装在键盘、触控板和额外电池一侧上，如果把该侧移除变成平板形态，独显就也一起没了\n"
			"        *2 比如 Windows 系统玄学 Bug"
		);
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

fResult f2dRenderDevice11::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL)
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
			if (FAILED(hr))
			{
				return FCYERR_INTERNALERR;
			}

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
			if (FAILED(hr))
			{
				return FCYERR_INTERNALERR;
			}

			// 检查刷新率，如果太低的话就继续往上匹配
			UINT const numerator_candidate[] = { 120, 180, 240, 360 };
			for (auto const& v : numerator_candidate)
			{
				if (((double)mode.RefreshRate.Numerator / (double)mode.RefreshRate.Denominator) > 59.5f)
				{
					break;
				}
				target_mode.RefreshRate = DXGI_RATIONAL{ .Numerator = v,.Denominator = 1 };
				hr = gHR = dxgi_output->FindClosestMatchingMode(&target_mode, &mode, d3d11_device.Get());
				if (FAILED(hr))
				{
					return FCYERR_INTERNALERR;
				}
			}

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
		return FCYERR_INVAILDPARAM;
	}
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRateA, fuInt RefreshRateB, fBool Windowed, fBool VSync, fBool FlipModel)
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
					.RefreshRate = DXGI_RATIONAL{.Numerator = RefreshRateA,.Denominator = RefreshRateB},
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

// 纹理读写

fResult f2dRenderDevice11::SaveScreen(f2dStream*) { return FCYERR_NOTSUPPORT; }
fResult f2dRenderDevice11::SaveTexture(f2dStream*, f2dTexture2D*) { return FCYERR_NOTSUPPORT; }
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
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDevice11::SaveScreen", "DirectX::SaveWICTextureToFile failed", hr));
		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

// 废弃的方法集合，大部分是固定管线遗毒，部分是完全没做好的功能，或者不适应新的图形API

fResult f2dRenderDevice11::SubmitWorldMat(const fcyMatrix4&)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitLookatMat(const fcyMatrix4&)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitProjMat(const fcyMatrix4&)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitBlendState(const f2dBlendState&)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitVD(void*)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SubmitTextureBlendOP_Color(int)
{
	return FCYERR_NOTIMPL;
}
F2DAALEVEL f2dRenderDevice11::GetAALevel()
{
	return F2DAALEVEL_NONE;
}
fBool f2dRenderDevice11::CheckMultiSample(F2DAALEVEL, fBool)
{
	return false;
}
fBool f2dRenderDevice11::IsZBufferEnabled()
{
	return false;
}
fResult f2dRenderDevice11::SetZBufferEnable(fBool)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::UpdateScreenToWindow(fcyColor, fByte)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetTextureAddress(F2DTEXTUREADDRESS, const fcyColor&)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::SetTextureFilter(F2DTEXFILTERTYPE)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateGraphics2D(fuInt, fuInt, f2dGraphics2D**)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateGraphics3D(f2dEffect*, f2dGraphics3D**)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateEffect(f2dStream*, fBool, f2dEffect**)
{
	return FCYERR_NOTIMPL;
}
fResult f2dRenderDevice11::CreateMeshData(f2dVertexElement*, fuInt, fuInt, fuInt, fBool, f2dMeshData**)
{
	return FCYERR_NOTIMPL;
}
