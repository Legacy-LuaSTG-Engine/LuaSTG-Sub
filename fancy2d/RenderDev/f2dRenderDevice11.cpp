#include "RenderDev/f2dRenderDevice11.h"

#include <fcyOS/fcyDebug.h>
#include "RenderDev/f2dTexture11.h"
#include "Engine/f2dEngineImpl.h"

#include "utility/encoding.hpp"
#include "platform/WindowsVersion.hpp"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/SwapChain_D3D11.hpp"

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
		count = std::snprintf(buffer, 64, "%.2f KiB", (double)size / 1024.0);
	}
	else if (size < (1024llu * 1024llu * 1024llu)) // MB
	{
		count = std::snprintf(buffer, 64, "%.2f MiB", (double)size / 1048576.0);
	}
	else // GB
	{
		count = std::snprintf(buffer, 64, "%.2f GiB", (double)size / 1073741824.0);
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
	win32_window = (HWND)pEngine->GetMainWindow()->GetHandle();

	spdlog::info("[fancy2d] 操作系统版本：{}", platform::WindowsVersion::GetName());
	get_system_memory_status();

	// 设备族

	if (!LuaSTG::Core::Graphics::IDevice::create(utility::encoding::to_utf8(RenderWindowParam->gpu ? RenderWindowParam->gpu : L""), ~m_pGraphicsDevice))
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "LuaSTG::Core::Graphics::IDevice::create failed");
	}

	auto* p_dev = dynamic_cast<LuaSTG::Core::Graphics::Device_D3D11*>(*m_pGraphicsDevice);

	dxgi_factory = p_dev->GetDXGIFactory1();
	dxgi_adapter = p_dev->GetDXGIAdapter1();

	d3d11_device = p_dev->GetD3D11Device();
	d3d11_devctx = p_dev->GetD3D11DeviceContext();
	
	tracy::xTracyD3D11Context(d3d11_device.Get(), d3d11_devctx.Get());

	// 交换链

	if (!LuaSTG::Core::Graphics::ISwapChain::create(win32_window, *m_pGraphicsDevice, ~m_pSwapChain))
	{
		throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::createSwapchain failed.");
	}
	if (RenderWindowParam->windowed)
	{
		auto& mode = RenderWindowParam->mode;
		m_pSwapChain->setVSync(RenderWindowParam->vsync);
		if (!m_pSwapChain->setWindowMode(mode.width, mode.height, false))
			throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::createSwapchain failed.");
	}
	else
	{
		m_pSwapChain->setVSync(RenderWindowParam->vsync);
		if (!m_pSwapChain->setExclusiveFullscreenMode(display_mode_to(RenderWindowParam->mode)))
			throw fcyException("f2dRenderDevice11::f2dRenderDevice11", "f2dRenderDevice11::createSwapchain failed.");
	}
	m_pSwapChain->refreshDisplayMode();

	m_pEngine->GetMainWindow()->SetGraphicListener(this);
	m_pGraphicsDevice->addEventListener(this);
	m_pSwapChain->addEventListener(this);
}
f2dRenderDevice11::~f2dRenderDevice11()
{
	m_pEngine->GetMainWindow()->SetGraphicListener(nullptr);
	m_pGraphicsDevice->removeEventListener(this);
	m_pSwapChain->removeEventListener(this);

	tracy::xTracyD3D11Destroy();

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
fcStr f2dRenderDevice11::GetDeviceName() { return dynamic_cast<LuaSTG::Core::Graphics::Device_D3D11*>(*m_pGraphicsDevice)->GetAdapterName().data(); }
fuInt f2dRenderDevice11::GetSupportedDeviceCount() { return dynamic_cast<LuaSTG::Core::Graphics::Device_D3D11*>(*m_pGraphicsDevice)->GetAdapterNameArray().size(); }
fcStr f2dRenderDevice11::GetSupportedDeviceName(fuInt Index) { return dynamic_cast<LuaSTG::Core::Graphics::Device_D3D11*>(*m_pGraphicsDevice)->GetAdapterNameArray()[Index].c_str(); }
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

void f2dRenderDevice11::onDeviceCreate()
{
	sendDevResetMsg();
}
void f2dRenderDevice11::onDeviceDestroy()
{
	sendDevLostMsg();
}

void f2dRenderDevice11::onSwapChainCreate()
{
	dispatchRenderSizeDependentResourcesCreate();
}
void f2dRenderDevice11::onSwapChainDestroy()
{
	dispatchRenderSizeDependentResourcesDestroy();
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
	// 处理窗口焦点
	if (swapchain_want_exit_fullscreen)
	{
		swapchain_want_exit_fullscreen = false;
		m_pSwapChain->onWindowInactive();
		m_pEngine->GetMainWindow()->SetTopMost(false);
	}
	else if (swapchain_want_enter_fullscreen)
	{
		swapchain_want_enter_fullscreen = false;
		m_pSwapChain->onWindowActive();
	}
	// 需要重新调整交换链大小
	if (swapchain_want_resize)
	{
		swapchain_want_resize = false;
		//if (!m_pSwapChain->setSize(uint32_t(-1), uint32_t(-1)))
		//{
		//	return FCYERR_INTERNALERR;
		//}
	}
	// 小 Hack，在这里绑定交换链的 RenderTarget
	m_pSwapChain->applyRenderAttachment();
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
		m_pSwapChain->applyRenderAttachment();
		ID3D11RenderTargetView* rtvs[1] = { NULL };
		ID3D11DepthStencilView* dsv = NULL;
		d3d11_devctx->OMGetRenderTargets(1, rtvs, &dsv);
		if (*m_RenderTarget)
		{
			if (rtvs[0]) rtvs[0]->Release();
			rtvs[0] = ((f2dRenderTarget11*)*m_RenderTarget)->GetRTView();
		}
		if (*m_DepthStencil)
		{
			if (dsv) dsv->Release();
			dsv = ((f2dDepthStencil11*)*m_DepthStencil)->GetView();
		}
		d3d11_devctx->OMSetRenderTargets(1, rtvs, dsv);
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

// 交换链

fuInt f2dRenderDevice11::GetSupportedDisplayModeCount(fBool refresh)
{
	if (m_pSwapChain->getDisplayModeCount() < 1 || refresh)
	{
		m_pSwapChain->refreshDisplayMode();
	}
	return m_pSwapChain->getDisplayModeCount();
}
f2dDisplayMode f2dRenderDevice11::GetSupportedDisplayMode(fuInt Index)
{
	return display_mode_from(m_pSwapChain->getDisplayMode(Index));
}
fResult f2dRenderDevice11::SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel)
{
	m_pSwapChain->setVSync(VSync);
	if (!m_pSwapChain->setWindowMode(Width, Height, FlipModel))
	{
		return FCYERR_INTERNALERR;
	}
	((f2dWindowImpl*)m_pEngine->GetMainWindow())->MoveMouseToRightBottom();
	return FCYERR_OK;
}
fResult f2dRenderDevice11::SetDisplayMode(f2dDisplayMode mode, fBool VSync)
{
	m_pSwapChain->setVSync(VSync);
	if (!m_pSwapChain->setExclusiveFullscreenMode(display_mode_to(mode)))
	{
		return FCYERR_INTERNALERR;
	}
	((f2dWindowImpl*)m_pEngine->GetMainWindow())->MoveMouseToRightBottom();
	return FCYERR_OK;
}
fuInt f2dRenderDevice11::GetBufferWidth() { return m_pSwapChain->getWidth(); }
fuInt f2dRenderDevice11::GetBufferHeight() { return m_pSwapChain->getHeight(); }
fBool f2dRenderDevice11::IsWindowed() { return m_pSwapChain->isWindowMode(); }
fResult f2dRenderDevice11::WaitDevice()
{
	m_pSwapChain->waitFrameLatency();
	return FCYERR_OK;
}
fResult f2dRenderDevice11::Present()
{
	m_RenderTarget = nullptr;
	m_DepthStencil = nullptr;
	if (!m_pSwapChain->present())
	{
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}

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

fResult f2dRenderDevice11::SaveScreen(f2dStream*) { return FCYERR_NOTSUPPORT; }
fResult f2dRenderDevice11::SaveTexture(f2dStream*, f2dTexture2D*) { return FCYERR_NOTSUPPORT; }
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
