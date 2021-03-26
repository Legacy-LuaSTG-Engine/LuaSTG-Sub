#include "GraphicDevice11/f2dRenderDeviceImpl.h"

#include <fcyMisc/fcyStringHelper.h>
#include <fcyMisc/fcyHash.h>
#include <fcyOS/fcyDebug.h>

#include "GraphicDevice11/f2dTextureImpl.h"
#include "GraphicDevice11/f2dGraphics2DImpl.h"

#include "Engine/f2dEngineImpl.h"

#include <algorithm>

#ifdef max
#undef max // FUCK YOU!
#endif

////////////////////////////////////////////////////////////////////////////////

f2dRenderDeviceImpl::VertexDeclareInfo::VertexDeclareInfo()
	: Hash(0), pVertexDeclare(NULL)
{}

f2dRenderDeviceImpl::VertexDeclareInfo::VertexDeclareInfo(const VertexDeclareInfo& Org)
	: Hash(Org.Hash), ElementData(Org.ElementData), pVertexDeclare(Org.pVertexDeclare)
{
	if(pVertexDeclare)
		pVertexDeclare->AddRef();
}

f2dRenderDeviceImpl::VertexDeclareInfo::~VertexDeclareInfo()
{
	FCYSAFEKILL(pVertexDeclare);
}

////////////////////////////////////////////////////////////////////////////////

static const D3DDISPLAYMODEFILTER g_d3d9DisplayModeFilter = {
	sizeof(D3DDISPLAYMODEFILTER),
	D3DFMT_X8R8G8B8,
	D3DSCANLINEORDERING_PROGRESSIVE,
};

f2dRenderDeviceImpl::f2dRenderDeviceImpl(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel) :
	m_pEngine(pEngine),
	m_pD3D9(NULL), m_pDev(NULL), m_hWnd(NULL),
	m_bDevLost(false), m_pBackBuffer(NULL), m_pBackDepthBuffer(NULL), m_pCurGraphics(NULL),
	m_pWinSurface(NULL), m_pCurBackBuffer(NULL), m_pCurBackDepthBuffer(NULL),
	m_pCurVertDecl(NULL), m_CreateThreadID(GetCurrentThreadId()), m_bZBufferEnabled(true)
{
	m_hWnd = (HWND)pEngine->GetMainWindow()->GetHandle();
	
	HRESULT hr = S_OK;
	
	// ---------- 创建 DXGI 工厂
	
	// 创建 DXGI 工厂
	hr = ::CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory1.GetAddressOf()));
	if (hr != S_OK)
	{
		debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] CreateDXGIFactory1 failed.");
		throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "CreateDXGIFactory1 Failed.", hr);
	}
	
	// 查询新接口
	hr = dxgiFactory1.As(&dxgiFactory2);
	if (hr != S_OK)
	{
		OutputDebugStringW(L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] QueryInterface IDXGIFactory2 failed.\n");
	}
	
	// 枚举显示适配器
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1_;
	Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6_;
	hr = dxgiFactory1.As(&dxgiFactory6_);
	if (hr == S_OK)
	{
		hr = dxgiFactory6_->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(dxgiAdapter1_.GetAddressOf()));
		if (hr != S_OK)
		{
			debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] IDXGIFactory6::EnumAdapterByGpuPreference failed.");
		}
	}
	else
	{
		OutputDebugStringW(L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] QueryInterface IDXGIFactory6 failed.\n");
	}
	if (!dxgiAdapter1_)
	{
		hr = dxgiFactory1->EnumAdapters1(0, dxgiAdapter1_.GetAddressOf());
		if (hr != S_OK)
		{
			debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] IDXGIFactory1::EnumAdapters1 failed.");
			throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "IDXGIFactory1::EnumAdapters1 failed.", hr);
		}
	}
	
	// 获取显示适配器名
	{
		DXGI_ADAPTER_DESC1 desc_ = {};
		hr = dxgiAdapter1_->GetDesc1(&desc_);
		if (hr == S_OK)
		{
			m_DevName = fcyStringHelper::WideCharToMultiByte(desc_.Description, CP_UTF8);
		}
		else
		{
			debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] IDXGIAdapter1::GetDesc1 failed.");
		}
	};
	
	// ---------- 创建 Direct3D11
	
	// 创建 Direct3D11
	UINT d3d11Flags_ = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	#ifdef _DEBUG
	d3d11Flags_ |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	const D3D_FEATURE_LEVEL target_levels[4] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_9_1;
	hr = ::D3D11CreateDevice(
		dxgiAdapter1_.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
		d3d11Flags_, target_levels, 4, D3D11_SDK_VERSION,
		d3d11Device.GetAddressOf(), &feature_level, d3d11DeviceContext.GetAddressOf());
	if (hr != S_OK)
	{
		debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] D3D11CreateDevice failed.");
		hr = ::D3D11CreateDevice(
			dxgiAdapter1_.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
			d3d11Flags_, target_levels + 1, 3, D3D11_SDK_VERSION,
			d3d11Device.GetAddressOf(), &feature_level, d3d11DeviceContext.GetAddressOf());
		if (hr != S_OK)
		{
			debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] D3D11CreateDevice failed.");
			throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "D3D11CreateDevice failed.", hr);
		}
	}
	
	// 查询新接口
	hr = d3d11Device.As(&d3d11Device1);
	if (hr != S_OK)
	{
		OutputDebugStringW(L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] QueryInterface ID3D11Device1 failed.\n");
	}
	hr = d3d11DeviceContext.As(&d3d11DeviceContext1);
	if (hr != S_OK)
	{
		OutputDebugStringW(L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] QueryInterface ID3D11DeviceContext1 failed.\n");
	}
	
	// ---------- 创建 交换链
	
	// 创建 交换链
	if (dxgiFactory2 && d3d11Device1 && d3d11DeviceContext1)
	{
		DXGI_SWAP_CHAIN_DESC1 info = {}; {
			info.Width = 640;
			info.Height = 480;
			info.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			info.Stereo = FALSE;
			info.SampleDesc.Count = 1;
			info.SampleDesc.Quality = 0;
			info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			info.BufferCount = 2;
			info.Scaling = DXGI_SCALING_STRETCH;
			info.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			info.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			//info.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
			//info.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		};
		hr = dxgiFactory2->CreateSwapChainForHwnd(d3d11Device1.Get(), m_hWnd, &info, NULL, NULL, dxgiSwapChain1.GetAddressOf());
		if (hr == S_OK)
		{
			hr = dxgiSwapChain1.As(&dxgiSwapChain);
			if (hr != S_OK)
			{
				OutputDebugStringW(L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] QueryInterface IDXGISwapChain failed.\n");
				throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "QueryInterface IDXGISwapChain failed.", hr);
			}
		}
		else
		{
			debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] IDXGIFactory2::CreateSwapChainForHwnd failed.");
		}
	}
	if (!dxgiSwapChain1)
	{
		DXGI_SWAP_CHAIN_DESC info = {}; {
			info.BufferDesc.Width = 640;
			info.BufferDesc.Height = 480;
			info.BufferDesc.RefreshRate.Numerator = 0;
			info.BufferDesc.RefreshRate.Denominator = 1;
			info.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			info.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			info.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			info.SampleDesc.Count = 1;
			info.SampleDesc.Quality = 0;
			info.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			info.BufferCount = 2;
			info.OutputWindow = m_hWnd;
			info.Windowed = TRUE;
			info.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			info.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		};
		hr = dxgiFactory1->CreateSwapChain(d3d11Device.Get(), &info, dxgiSwapChain.GetAddressOf());
		if (hr != S_OK)
		{
			debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::f2dRenderDeviceImpl] IDXGIFactory1::CreateSwapChain failed.");
			throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "IDXGIFactory1::CreateSwapChain failed.", hr);
		}
	}
}

f2dRenderDeviceImpl::~f2dRenderDeviceImpl()
{
	// ---------- 删除渲染器监听链
	
	for (auto& v : _setEventListeners)
	{
		// 报告可能的对象泄漏
		char tTextBuffer[256];
		sprintf_s(tTextBuffer, "Unrelease listener object at %p", v.listener);
#ifdef _DEBUG
		fcyDebug::Trace("[ @ f2dRenderDeviceImpl::~f2dRenderDeviceImpl ] %s\n", tTextBuffer);
#endif
		m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::~f2dRenderDeviceImpl", tTextBuffer));
	}
	_setEventListeners.clear();
	
	// ---------- 清理状态
	
	// 清理状态
	if (d3d11DeviceContext)
	{
		d3d11DeviceContext->ClearState();
	}
	
	// ---------- 删除组件
	
	// 交换链资源
	d3d11BackBuffer.Reset();
	d3d11DepthStencil.Reset();
	dxgiSwapChain.Reset();
	dxgiSwapChain1.Reset();
	// Direct3D11
	d3d11Device.Reset();
	d3d11DeviceContext.Reset();
	d3d11Device1.Reset();
	d3d11DeviceContext1.Reset();
	// DXGI
	dxgiFactory1.Reset();
	dxgiFactory2.Reset();
}

HRESULT f2dRenderDeviceImpl::doReset()
{
	struct Delegate : public f2dMainThreadDelegate
	{
		IDirect3DDevice9Ex* DEV;
		D3DPRESENT_PARAMETERS D3DPP;
		D3DDISPLAYMODEEX D3DPPEX;
		HRESULT HR;
		
		virtual void AddRef() {}
		virtual void Release() {}
		void Excute() {
			HR = DEV->ResetEx(&D3DPP, D3DPP.Windowed ? NULL : &D3DPPEX);
		}
		
		Delegate(IDirect3DDevice9Ex* device, D3DPRESENT_PARAMETERS& pp, D3DDISPLAYMODEEX& ppex) :
			DEV(device), D3DPP(pp), D3DPPEX(ppex), HR(D3D_OK) {}
	};
	
	HRESULT hr = S_OK;
	if(GetCurrentThreadId() != m_CreateThreadID)
	{
		Delegate obj(_d3d9DeviceEx.Get(), _d3d9SwapChainInfo, _d3d9FullScreenSwapChainInfo);
		m_pEngine->InvokeDelegateAndWait(&obj);
		if (obj.HR != D3D_OK)
		{
			m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::~doReset", "IDirect3DDevice9Ex::ResetEx failed (HRESULT = 0x%08X).", obj.HR));
		}
		hr = obj.HR;
	}
	else
	{
		D3DPRESENT_PARAMETERS D3DPP = _d3d9SwapChainInfo;
		D3DDISPLAYMODEEX D3DPPEX = _d3d9FullScreenSwapChainInfo;
		hr = _d3d9DeviceEx->ResetEx(&D3DPP, D3DPP.Windowed ? NULL : &D3DPPEX);
	}
	return hr;
}

int f2dRenderDeviceImpl::sendDevLostMsg()
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

int f2dRenderDeviceImpl::sendDevResetMsg()
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

int f2dRenderDeviceImpl::dispatchRenderSizeDependentResourcesCreate()
{
	int cnt = 0;
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesCreate();
		cnt += 1;
	}
	return cnt;
}

int f2dRenderDeviceImpl::dispatchRenderSizeDependentResourcesDestroy()
{
	int cnt = 0;
	for (auto& v : _setEventListeners)
	{
		v.listener->OnRenderSizeDependentResourcesDestroy();
		cnt += 1;
	}
	return cnt;
}

fResult f2dRenderDeviceImpl::SyncDevice()
{
	// 需要重置设备
	if (m_bDevLost)
	{
		HRESULT hr = S_OK;
		if (hr == S_OK)
		{
			// 他说可以了
			hr = doReset();
			if (hr == S_OK)
			{
				m_bDevLost = false;
				int tObjCount = sendDevResetMsg(); // 通知非托管组件恢复工作
				char tBuffer[256] = {};
				snprintf(tBuffer, 255, "Device reseted. ( %d Object(s) rested. )", tObjCount);
				m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::SyncDevice", tBuffer));
				return FCYERR_OK;
			}
		}
		return FCYERR_INTERNALERR;
	}
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel)
{
	HRESULT hr = S_OK;
	
	// 检查参数
	if (Width < 1 || Height < 1)
		return FCYERR_INVAILDPARAM;
	
	// 通知一些资源需要释放
	dispatchRenderSizeDependentResourcesDestroy();
	// 移除旧的后备缓冲区
	d3d11BackBuffer.Reset();
	
	// 修改交换链大小
	if (dxgiSwapChain1)
	{
		DXGI_SWAP_CHAIN_DESC1 lastinfo = {};
		hr = dxgiSwapChain1->GetDesc1(&lastinfo);
		hr = dxgiSwapChain1->ResizeBuffers(lastinfo.BufferCount, Width, Height, lastinfo.Format, lastinfo.Flags);
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC lastinfo = {};
		hr = dxgiSwapChain->GetDesc(&lastinfo);
		hr = dxgiSwapChain->ResizeBuffers(lastinfo.BufferCount, Width, Height, lastinfo.BufferDesc.Format, lastinfo.Flags);
	}
	if (hr != S_OK)
	{
		debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::SetBufferSize] IDXGISwapChain ResizeBuffers failed.");
		// 设备丢失
		isDeviceLost = true; // 标记为设备丢失状态
		// 广播设备丢失事件
		int tObjCount = sendDevLostMsg();
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "Detected device lost. ( %d Object(s) losted. )", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::SetBufferSize", tBuffer));
		return hr == FCYERR_INTERNALERR;
	}
	
	// 重新创建缓冲区
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
	hr = dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(texture_.GetAddressOf()));
	hr = d3d11Device->CreateRenderTargetView(texture_.Get(), NULL, d3d11BackBuffer.GetAddressOf());
	
	// 恢复资源
	dispatchRenderSizeDependentResourcesCreate();
	
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::Present()
{
	HRESULT hr = S_OK;
	
	// 还原当前的后台缓冲区
	ID3D11RenderTargetView* const backbuffers_[1] = { d3d11BackBuffer.Get() };
	d3d11DeviceContext->OMSetRenderTargets(1, backbuffers_, d3d11DepthStencil.Get());
	
	// 呈现
	hr = dxgiSwapChain->Present(0, 0);
	if (hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		debugPrintHRESULT(hr, L"[f2dRenderDeviceImpl::Present] IDXGISwapChain Present failed.");
		// 设备丢失
		isDeviceLost = true; // 标记为设备丢失状态
		// 广播设备丢失事件
		int tObjCount = sendDevLostMsg();
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "Detected device lost. ( %d Object(s) losted. )", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::Present", tBuffer));
	}
	
	return hr == S_OK ? FCYERR_OK : FCYERR_INTERNALERR;
}

fResult f2dRenderDeviceImpl::SubmitCurGraphics(f2dGraphics* pGraph, bool bDirty)
{
	if(pGraph == NULL)
	{
		m_pCurGraphics = NULL;
		return FCYERR_OK;
	}
	else if(m_pCurGraphics)
	{
		if(m_pCurGraphics->IsInRender())
			return FCYERR_ILLEGAL;
	}
	
	if(!bDirty && pGraph == m_pCurGraphics)
		return FCYERR_OK;
	
	m_pCurGraphics = pGraph;
	
	return FCYERR_OK;
}
fResult f2dRenderDeviceImpl::SubmitWorldMat(const fcyMatrix4& Mat)
{
	return FCYERR_OK;
}
fResult f2dRenderDeviceImpl::SubmitLookatMat(const fcyMatrix4& Mat)
{
	return FCYERR_OK;
}
fResult f2dRenderDeviceImpl::SubmitProjMat(const fcyMatrix4& Mat)
{
	return FCYERR_OK;
}
fResult f2dRenderDeviceImpl::SubmitBlendState(const f2dBlendState& State)
{
	return FCYERR_OK;
}
fResult f2dRenderDeviceImpl::SubmitVD(IDirect3DVertexDeclaration9* pVD)
{
	return FCYERR_OK;
}
fResult f2dRenderDeviceImpl::SubmitTextureBlendOP_Color(D3DTEXTUREOP ColorOP)
{
	return FCYERR_OK;
}

IDirect3DVertexDeclaration9* f2dRenderDeviceImpl::RegisterVertexDeclare(f2dVertexElement* pElement, fuInt ElementCount, fuInt& ElementSize)
{
	return NULL;
}

F2DAALEVEL f2dRenderDeviceImpl::GetAALevel()
{
	return F2DAALEVEL_NONE;
}

fBool f2dRenderDeviceImpl::CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed)
{
	return false;
}

fuInt f2dRenderDeviceImpl::GetSupportResolutionCount()
{
	return _d3d9Ex->GetAdapterModeCountEx(D3DADAPTER_DEFAULT, &g_d3d9DisplayModeFilter);
}

fcyVec2 f2dRenderDeviceImpl::EnumSupportResolution(fuInt Index)
{
	fcyVec2 tRet;
	D3DDISPLAYMODEEX mode = {};
	mode.Size = sizeof(D3DDISPLAYMODEEX);
	
	if (D3D_OK == _d3d9Ex->EnumAdapterModesEx(D3DADAPTER_DEFAULT, &g_d3d9DisplayModeFilter, Index, &mode))
	{
		tRet.x = (float)mode.Width;
		tRet.y = (float)mode.Height;
	}
	
	return tRet;
}

fResult f2dRenderDeviceImpl::AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority)
{
	if(Listener == NULL)
		return FCYERR_INVAILDPARAM;
	
	EventListenerNode node;
	node.uuid = _iEventListenerUUID;
	node.priority = Priority;
	node.listener = Listener;
	auto v = _setEventListeners.emplace(node);
	_iEventListenerUUID += 1;
	
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::RemoveListener(f2dRenderDeviceEventListener* Listener)
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

fResult f2dRenderDeviceImpl::CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		if(IsDynamic)
			*pOut = new f2dTexture2DDynamic(this, pStream, Width, Height);
		else
			*pOut = new f2dTexture2DStatic(this, pStream, Width, Height, HasMipmap);
	}
	catch (const std::bad_alloc&)
	{
		return FCYERR_OUTOFMEM;
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)
{
	if (!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		if (IsDynamic)
			*pOut = new f2dTexture2DDynamic(this, pMemory, Size, Width, Height);
		else
			*pOut = new f2dTexture2DStatic(this, pMemory, Size, Width, Height, HasMipmap);
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

fResult f2dRenderDeviceImpl::CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2DDynamic(this, Width, Height);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize, f2dTexture2D** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dTexture2DRenderTarget(this, Width, Height, AutoResize);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dDepthStencilSurfaceImpl(this, Width, Height, AutoResize, Discard);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	if(VertexBufferSize == 0)
		VertexBufferSize = 512;
	if(IndexBufferSize == 0)
		IndexBufferSize = 768;

	try
	{
		*pOut = new f2dGraphics2DImpl(this, VertexBufferSize, IndexBufferSize);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::Clear(const fcyColor& BackBufferColor, fFloat ZValue)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	return FAILED(m_pDev->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, BackBufferColor.argb, ZValue, 0))?FCYERR_INTERNALERR:FCYERR_OK;
}

fResult f2dRenderDeviceImpl::Clear(const fcyColor& BackBufferColor, fFloat ZValue, fuInt StencilValue)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	return FAILED(m_pDev->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET | D3DCLEAR_STENCIL, BackBufferColor.argb, ZValue, StencilValue))?FCYERR_INTERNALERR:FCYERR_OK;
}

fResult f2dRenderDeviceImpl::ClearColor(const fcyColor& BackBufferColor)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	return FAILED(m_pDev->Clear(0, NULL, D3DCLEAR_TARGET, BackBufferColor.argb, 1.f, 0))?FCYERR_INTERNALERR:FCYERR_OK;
}

fResult f2dRenderDeviceImpl::ClearZBuffer(fFloat Value)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	return FAILED(m_pDev->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, Value, 0))?FCYERR_INTERNALERR:FCYERR_OK;
}

fResult f2dRenderDeviceImpl::ClearStencilBuffer(fuInt StencilValue)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	return FAILED(m_pDev->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0, StencilValue))?FCYERR_INTERNALERR:FCYERR_OK;
}

f2dTexture2D* f2dRenderDeviceImpl::GetRenderTarget()
{
	return m_pCurBackBuffer;
}

fResult f2dRenderDeviceImpl::SetRenderTarget(f2dTexture2D* pTex)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	if(m_pCurBackBuffer == pTex)
		return FCYERR_OK;

	if(!pTex)
	{
		FCYSAFEKILL(m_pCurBackBuffer);
		m_pDev->SetRenderTarget(0, m_pBackBuffer);

		// 此时vp已发生变化，跟踪新的vp
		m_pDev->GetViewport(&m_ViewPort);

		FCYSAFEKILL(m_pBackBuffer);
		return FCYERR_OK;
	}
	else if(!pTex->IsRenderTarget())
		return FCYERR_INVAILDPARAM;

	FCYSAFEKILL(m_pCurBackBuffer);
	m_pCurBackBuffer = pTex;
	m_pCurBackBuffer->AddRef();
	if(m_pBackBuffer == NULL)
		m_pDev->GetRenderTarget(0, &m_pBackBuffer);

	m_pDev->SetRenderTarget(0, ((f2dTexture2DRenderTarget*)m_pCurBackBuffer)->GetSurface());

	// 此时vp已发生变化，跟踪新的vp
	m_pDev->GetViewport(&m_ViewPort);

	return FCYERR_OK;
}

f2dDepthStencilSurface* f2dRenderDeviceImpl::GetDepthStencilSurface()
{
	return m_pCurBackDepthBuffer;
}

fResult f2dRenderDeviceImpl::SetDepthStencilSurface(f2dDepthStencilSurface* pSurface)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	if(m_pCurBackDepthBuffer == pSurface)
		return FCYERR_OK;

	if(!pSurface)
	{
		FCYSAFEKILL(m_pCurBackDepthBuffer);
		m_pDev->SetDepthStencilSurface(m_pBackDepthBuffer);
		FCYSAFEKILL(m_pBackDepthBuffer);
		return FCYERR_OK;
	}

	FCYSAFEKILL(m_pCurBackDepthBuffer);
	m_pCurBackDepthBuffer = pSurface;
	m_pCurBackDepthBuffer->AddRef();
	if(m_pBackDepthBuffer == NULL)
		m_pDev->GetDepthStencilSurface(&m_pBackDepthBuffer);

	m_pDev->SetDepthStencilSurface((IDirect3DSurface9*)pSurface->GetHandle());

	return FCYERR_OK;
}

fcyRect f2dRenderDeviceImpl::GetScissorRect()
{
	return fcyRect(
		(float)m_ScissorRect.left,
		(float)m_ScissorRect.top,
		(float)m_ScissorRect.right,
		(float)m_ScissorRect.bottom
	);
}

fResult f2dRenderDeviceImpl::SetScissorRect(const fcyRect& pRect)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();
	
	const LONG	_l = (LONG)pRect.a.x,
				_t = (LONG)pRect.a.y,
				_r = (LONG)pRect.b.x,
				_b = (LONG)pRect.b.y;
	
	if (_l != m_ScissorRect.left || _t != m_ScissorRect.top || _r != m_ScissorRect.right || _b != m_ScissorRect.bottom)
	{
		m_ScissorRect.left   = _l;
		m_ScissorRect.top    = _t;
		m_ScissorRect.right  = _r;
		m_ScissorRect.bottom = _b;
		if(FAILED(m_pDev->SetScissorRect(&m_ScissorRect)))
			return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
}

fcyRect f2dRenderDeviceImpl::GetViewport()
{
	return fcyRect(
		(float)m_ViewPort.X,
		(float)m_ViewPort.Y,
		(float)(m_ViewPort.X + m_ViewPort.Width),
		(float)(m_ViewPort.Y + m_ViewPort.Height)
	);
}

fResult f2dRenderDeviceImpl::SetViewport(fcyRect vp)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	//if (!vp.Intersect(fcyRect(0, 0, (float)GetBufferWidth(), (float)GetBufferHeight()), &vp))
	//	return FCYERR_ILLEGAL;
	
	// 限制范围
	//const float sWidth = (float)GetBufferWidth();
	//const float sHeight = (float)GetBufferHeight();
	//vp.a.x = std::clamp(vp.a.x, 0.0f, sWidth);
	//vp.b.x = std::clamp(vp.b.x, 0.0f, sWidth);
	//vp.a.y = std::clamp(vp.a.y, 0.0f, sHeight);
	//vp.b.y = std::clamp(vp.b.y, 0.0f, sHeight);
	vp.a.x = std::max(0.0f, vp.a.x);
	vp.a.y = std::max(0.0f, vp.a.y);
	vp.b.x = std::max(vp.a.x + 1.0f, vp.b.x);
	vp.b.y = std::max(vp.a.y + 1.0f, vp.b.y);
	
	const DWORD	_x = (DWORD)vp.a.x,
				_y = (DWORD)vp.a.y,
				_w = (DWORD)vp.GetWidth(),
				_h = (DWORD)vp.GetHeight();
	
	if (_x != m_ViewPort.X || _y != m_ViewPort.Y || _w != m_ViewPort.Width || _h != m_ViewPort.Height)
	{
		m_ViewPort.X      = _x;
		m_ViewPort.Y      = _y;
		m_ViewPort.Width  = _w;
		m_ViewPort.Height = _h;
		if (FAILED(m_pDev->SetViewport(&m_ViewPort)))
			return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
}

fBool f2dRenderDeviceImpl::IsZBufferEnabled()
{
	return m_bZBufferEnabled;
}

fResult f2dRenderDeviceImpl::SetZBufferEnable(fBool v)
{
	m_bZBufferEnabled = v;
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor) {
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SetTextureFilter(F2DTEXFILTERTYPE filter) {
	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

fResult f2dRenderDeviceImpl::SaveScreen(fcStrW path)
{
	return FCYERR_NOTSUPPORT;
}

fResult f2dRenderDeviceImpl::SaveTexture(fcStrW path, f2dTexture2D* pTex)
{
	return FCYERR_NOTSUPPORT;
}
