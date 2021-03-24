#include "RenderDev/f2dRenderDeviceImpl.h"

#include <fcyMisc/fcyStringHelper.h>
#include <fcyMisc/fcyHash.h>
#include <fcyOS/fcyDebug.h>

#include "RenderDev/f2dTextureImpl.h"
#include "RenderDev/f2dGraphics2DImpl.h"
#include "RenderDev/f2dGraphics3DImpl.h"
#include "RenderDev/f2dEffectImpl.h"
#include "RenderDev/f2dMeshDataImpl.h"

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

static const D3DPRESENT_PARAMETERS gc_d3d9TestSwapChainDesc = {
	// 测试用最小交换链(1x1)，窗口模式下，应该自动选择交换链格式
	1, 1, D3DFMT_UNKNOWN, 1,
	// 多重采样必须是关的
	D3DMULTISAMPLE_NONE, 0,
	// 窗口模式，传统交换链模型
	D3DSWAPEFFECT_DISCARD, NULL, TRUE,
	// 因为是测试目的，所以不用开自动深度缓冲区
	FALSE, D3DFMT_UNKNOWN,
	// 其他
	0,
	// 显示器，窗口模式下刷新率必须为0
	0, D3DPRESENT_INTERVAL_IMMEDIATE,
};

bool _findMatchDisplayMode(IDirect3D9Ex* d3d9, D3DDISPLAYMODEEX& mode, UINT width, UINT height)
{
	// 清空
	ZeroMemory(&mode, sizeof(D3DDISPLAYMODEEX));
	mode.Size = sizeof(D3DDISPLAYMODEEX);
	
	// 枚举
	UINT modecnt = d3d9->GetAdapterModeCountEx(D3DADAPTER_DEFAULT, &g_d3d9DisplayModeFilter);
	std::vector<D3DDISPLAYMODEEX> modes(modecnt);
	for (UINT idx = 0; idx < modecnt; idx += 1)
	{
		ZeroMemory(&modes[idx], sizeof(D3DDISPLAYMODEEX));
		modes[idx].Size = sizeof(D3DDISPLAYMODEEX);
		d3d9->EnumAdapterModesEx(D3DADAPTER_DEFAULT, &g_d3d9DisplayModeFilter, idx, &modes[idx]);
	}
	
	// 匹配合适的模式
	D3DDISPLAYMODEEX curmode = {};
	bool findmode = false;
	// 预处理，筛掉大小不一样的模式
	for (auto it = modes.begin(); it != modes.end();)
	{
		if (it->Width != width || it->Height != height)
			it = modes.erase(it);
		else
			it++;
	}
	// 第一轮寻找，查找60Hz或者60Hz倍数的刷新率（最适合的模式）
	ZeroMemory(&curmode, sizeof(D3DDISPLAYMODEEX));
	for (auto& v : modes)
	{
		if (v.RefreshRate >= 60 && (v.RefreshRate % 60) == 0)
		{
			if (curmode.RefreshRate < v.RefreshRate)
			{
				curmode = v;
				findmode = true;
			}
		}
	}
	if (findmode)
	{
		mode = curmode;
		return true;
	}
	// 第二轮寻找，查找接近60Hz或者接近120Hz的刷新率（有一些误差的模式）
	ZeroMemory(&curmode, sizeof(D3DDISPLAYMODEEX));
	for (auto& v : modes)
	{
		if ((v.RefreshRate >= 59 && v.RefreshRate <= 61) || (v.RefreshRate >= 118 && v.RefreshRate <= 122))
		{
			if (curmode.RefreshRate < v.RefreshRate)
			{
				curmode = v;
				findmode = true;
			}
		}
	}
	if (findmode)
	{
		mode = curmode;
		return true;
	}
	// 第三轮寻找，查找大于等于120Hz的刷新率（可以接受的模式）
	ZeroMemory(&curmode, sizeof(D3DDISPLAYMODEEX));
	for (auto& v : modes)
	{
		if (v.RefreshRate >= 120)
		{
			if (curmode.RefreshRate < v.RefreshRate)
			{
				curmode = v;
				findmode = true;
			}
		}
	}
	if (findmode)
	{
		mode = curmode;
		return true;
	}
	// 第四轮寻找，查找剩下的刷新率最高的模式（可能会导致画面不流畅或帧率较低的模式）
	ZeroMemory(&curmode, sizeof(D3DDISPLAYMODEEX));
	for (auto& v : modes)
	{
		if (curmode.RefreshRate < v.RefreshRate)
		{
			curmode = v;
			findmode = true;
		}
	}
	if (findmode)
	{
		mode = curmode;
		return true;
	}
	
	// 依然没找到
	return false;
};

f2dRenderDeviceImpl::f2dRenderDeviceImpl(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel) :
	m_pEngine(pEngine),
	m_pD3D9(NULL), m_pDev(NULL), m_hWnd(NULL),
	m_bDevLost(false), m_pBackBuffer(NULL), m_pBackDepthBuffer(NULL), m_pCurGraphics(NULL),
	m_pWinSurface(NULL), m_pCurBackBuffer(NULL), m_pCurBackDepthBuffer(NULL),
	m_pCurVertDecl(NULL), m_CreateThreadID(GetCurrentThreadId()), m_bZBufferEnabled(true)
{
	HRESULT hr = S_OK;
	m_hWnd = (HWND)pEngine->GetMainWindow()->GetHandle();
	
	// --- 创建D3D9 ---
	
	hr = Direct3DCreate9Ex(D3D_SDK_VERSION, _d3d9Ex.GetAddressOf());
	if (hr != S_OK)
	{
		throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "Direct3DCreate9Ex Failed.", hr);
	}
	hr = _d3d9Ex.As(&_d3d9);
	if (hr != S_OK)
	{
		throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "Query interface IDirect3D9 Failed.", hr);
	}
	m_pD3D9 = _d3d9.Get();
	
	// --- 测试 ---
	
	// 检查是否支持硬件顶点处理
	{
		D3DPRESENT_PARAMETERS testinfo = gc_d3d9TestSwapChainDesc;
		testinfo.hDeviceWindow = m_hWnd;
		hr = _d3d9Ex->CreateDeviceEx(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_hWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
			&testinfo,
			NULL,
			_d3d9DeviceEx.GetAddressOf());
		_d3d9SupportHWVertex = (hr == S_OK);
		_d3d9DeviceEx.Reset();
	};
	
	// 顶点处理的flag，这个变量一会会频繁用到，启动多线程 + 自动决定顶点处理
	DWORD BehaviorFlags = D3DCREATE_MULTITHREADED
						| (_d3d9SupportHWVertex ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);
	
	// 检查是否支持FLIPEX
	{
		D3DPRESENT_PARAMETERS testinfo = gc_d3d9TestSwapChainDesc;
		//testinfo.BackBufferCount = 2;
		testinfo.SwapEffect = D3DSWAPEFFECT_FLIPEX;
		testinfo.hDeviceWindow = m_hWnd;
		hr = _d3d9Ex->CreateDeviceEx(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_hWnd,
			BehaviorFlags,
			&testinfo,
			NULL,
			_d3d9DeviceEx.GetAddressOf());
		_d3d9SupportFlip = (hr == S_OK);
		_d3d9DeviceEx.Reset();
	};
	
	// --- 枚举可用的全屏模式 ---
	
	// 有没有找到？
	if (!_findMatchDisplayMode(_d3d9Ex.Get(), _d3d9FullScreenSwapChainInfo, BackBufferWidth, BackBufferHeight))
	{
		if (!Windowed) // 想全屏，但是没匹配的模式
		{
			throw fcyException("f2dRenderDeviceImpl::f2dRenderDeviceImpl",
				"Cannot find a valid full-screen display mode (%ux%u).", BackBufferWidth, BackBufferHeight);
		}
	}
	
	// --- 创建设备 ---
	
	// 填充信息
	ZeroMemory(&_d3d9SwapChainInfo, sizeof(D3DPRESENT_PARAMETERS));
	auto& winfo = _d3d9SwapChainInfo; {
		winfo.BackBufferWidth            = BackBufferWidth;
		winfo.BackBufferHeight           = BackBufferHeight;
		winfo.BackBufferFormat           = D3DFMT_UNKNOWN; // 窗口模式下，应该自动选择交换链格式
		winfo.BackBufferCount            = 1;
		winfo.MultiSampleType            = D3DMULTISAMPLE_NONE;
		winfo.MultiSampleQuality         = 0;
		winfo.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
		winfo.hDeviceWindow              = m_hWnd;
		winfo.Windowed                   = Windowed;
		winfo.EnableAutoDepthStencil     = TRUE;
		winfo.AutoDepthStencilFormat     = D3DFMT_D24S8;
		winfo.Flags                      = 0;
		winfo.FullScreen_RefreshRateInHz = 0;
		winfo.PresentationInterval       = VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	};
	
	// 创建
	{
		hr = _d3d9Ex->CreateDeviceEx(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_hWnd,
			BehaviorFlags,
			&_d3d9SwapChainInfo,
			Windowed ? NULL : &_d3d9FullScreenSwapChainInfo, // 窗口模式下必须为NULL
			_d3d9DeviceEx.GetAddressOf());
		if(hr != S_OK)
		{
			throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "IDirect3D9Ex::CreateDeviceEx Failed.", hr);
		}
		hr = _d3d9DeviceEx.As(&_d3d9Device);
		if (hr != S_OK)
		{
			throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "Query interface IDirect3DDevice9 Failed.", hr);
		}
		m_pDev = _d3d9Device.Get();
		// 设置帧延迟
		_d3d9DeviceEx->SetMaximumFrameLatency(1);
		// 创建同步设施
		_d3d9DeviceTest = DeviceSyncTest(m_hWnd, _d3d9DeviceEx.Get());
	};
	
	// --- 获取设备参数 ---
	
	// 清空
	D3DADAPTER_IDENTIFIER9 tIdentify;
	ZeroMemory(&tIdentify, sizeof(D3DADAPTER_IDENTIFIER9));
	// 获取
	hr = _d3d9Ex->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &tIdentify);
	if (hr == D3D_OK)
	{
		m_DevName = tIdentify.Description;
	}
	
	// --- 初始化渲染状态 ---
	
	m_ViewPort.X      = 0;
	m_ViewPort.Y      = 0;
	m_ViewPort.Width  = GetBufferWidth();
	m_ViewPort.Height = GetBufferHeight();
	m_ViewPort.MinZ   = 0.0f;
	m_ViewPort.MaxZ   = 1.0f;
	
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = GetBufferWidth();
	m_ScissorRect.bottom = GetBufferHeight();
	
	m_CurBlendState.BlendOp        = F2DBLENDOPERATOR_ADD;
	m_CurBlendState.SrcBlend       = F2DBLENDFACTOR_SRCALPHA;
	m_CurBlendState.DestBlend      = F2DBLENDFACTOR_INVSRCALPHA;
	m_CurBlendState.AlphaBlendOp   = F2DBLENDOPERATOR_ADD;
	m_CurBlendState.AlphaSrcBlend  = F2DBLENDFACTOR_SRCALPHA;
	m_CurBlendState.AlphaDestBlend = F2DBLENDFACTOR_INVSRCALPHA;
	
	m_CurTexBlendOP_Color = D3DTOP_ADD;
	
	m_CurWorldMat  = fcyMatrix4::GetIdentity();
	m_CurLookatMat = fcyMatrix4::GetIdentity();
	m_CurProjMat   = fcyMatrix4::GetIdentity();
	
	initState();
}

f2dRenderDeviceImpl::~f2dRenderDeviceImpl()
{
	// 释放对象
	FCYSAFEKILL(m_pCurBackBuffer);
	FCYSAFEKILL(m_pCurBackDepthBuffer);
	FCYSAFEKILL(m_pBackBuffer);
	FCYSAFEKILL(m_pBackDepthBuffer);
	
	// 删除渲染器监听链
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
	
	// 释放顶点声明
	m_VDCache.clear();
	
	// 释放DX组件
	FCYSAFEKILL(m_pWinSurface);
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

HRESULT f2dRenderDeviceImpl::doTestCooperativeLevel()
{
	if(GetCurrentThreadId() != m_CreateThreadID)
	{
		_d3d9DeviceTest.Reset();
		m_pEngine->InvokeDelegateAndWait(&_d3d9DeviceTest);
		return _d3d9DeviceTest.GetResult();
	}
	else
	{
		return _d3d9DeviceEx->CheckDeviceState(m_hWnd);
	}
}

void f2dRenderDeviceImpl::initState()
{
	auto* ctx = _d3d9DeviceEx.Get();
	
	// --- 初始化视口和裁剪矩形 ---
	
	ctx->SetViewport(&m_ViewPort);
	ctx->SetScissorRect(&m_ScissorRect);
	
	// --- 设置默认渲染状态 ---
	
	ctx->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);  // 设置反面剔除
	ctx->SetRenderState(D3DRS_LIGHTING, FALSE);         // 关闭光照
	ctx->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE); // 打开矩形裁剪功能
	
	// --- 设置ZBUFFER ---
	
	ctx->SetRenderState(D3DRS_ZENABLE, m_bZBufferEnabled);
	ctx->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	ctx->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	
	// --- 设置默认混合状态 ---
	
	ctx->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	ctx->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	
	ctx->SetRenderState(D3DRS_BLENDOP, m_CurBlendState.BlendOp);
	ctx->SetRenderState(D3DRS_SRCBLEND, m_CurBlendState.SrcBlend);
	ctx->SetRenderState(D3DRS_DESTBLEND, m_CurBlendState.DestBlend);
	ctx->SetRenderState(D3DRS_BLENDOPALPHA, m_CurBlendState.AlphaBlendOp);
	ctx->SetRenderState(D3DRS_SRCBLENDALPHA, m_CurBlendState.AlphaSrcBlend);
	ctx->SetRenderState(D3DRS_DESTBLENDALPHA, m_CurBlendState.AlphaDestBlend);
	
	// --- 纹理混合参数默认值 ---
	
	ctx->SetTextureStageState(0, D3DTSS_COLOROP,  m_CurTexBlendOP_Color);    
	//ctx->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	//ctx->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	ctx->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	ctx->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//手动premul需要把texture放在第一个乘以texture的alpha
	ctx->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_MODULATE);
	ctx->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	ctx->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	
	// --- 设置采样器 ---
	
	ctx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	ctx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	ctx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	ctx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	ctx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	
	// --- 设置默认变换矩阵 ---
	
	ctx->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m_CurWorldMat);
	ctx->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_CurLookatMat);
	ctx->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_CurProjMat);
}

int f2dRenderDeviceImpl::sendDevLostMsg()
{
	int tRet = 0;
	
	// 释放可能的对象
	m_pCurVertDecl = NULL;
	FCYSAFEKILL(m_pCurBackBuffer);
	FCYSAFEKILL(m_pCurBackDepthBuffer);
	FCYSAFEKILL(m_pBackBuffer);
	FCYSAFEKILL(m_pBackDepthBuffer);
	
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
		HRESULT hr = doTestCooperativeLevel();
		if (hr == S_OK)
		{
			// 他说可以了
			hr = doReset();
			if (hr == S_OK)
			{
				m_bDevLost = false;
				initState(); // 重新初始化状态
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

fResult f2dRenderDeviceImpl::Present()
{
	// 还原当前的后台缓冲区
	if(m_pBackBuffer)
	{
		_d3d9DeviceEx->SetRenderTarget(0, m_pBackBuffer);
		_d3d9DeviceEx->GetViewport(&m_ViewPort); // 切换渲染目标会导致视口变化，这里要刷新视口
		FCYSAFEKILL(m_pBackBuffer);
		FCYSAFEKILL(m_pCurBackBuffer);
	}
	if(m_pBackDepthBuffer)
	{
		_d3d9DeviceEx->SetDepthStencilSurface(m_pBackDepthBuffer);
		FCYSAFEKILL(m_pBackDepthBuffer);
		FCYSAFEKILL(m_pCurBackDepthBuffer);
	}
	
	// 呈现
	HRESULT hr = _d3d9DeviceEx->PresentEx(NULL, NULL, NULL, NULL, 0);
	//if (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICEHUNG || hr == D3DERR_DEVICEREMOVED)
	if (hr != S_OK && hr != S_PRESENT_OCCLUDED && hr != S_PRESENT_MODE_CHANGED)
	{
		// 设备丢失，广播设备丢失事件
		m_bDevLost = true; // 标记为设备丢失状态
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

	// 更新状态
	if(!pGraph->IsGraphics3D())
	{
		SubmitWorldMat(pGraph->GetWorldTransform());
		SubmitLookatMat(pGraph->GetViewTransform());
		SubmitProjMat(pGraph->GetProjTransform());
		m_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);   // 新行为：若为2D渲染器，则关闭剔除
	}
	else
		m_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);   // 新行为：若为3D渲染器，则启用逆时针剔除

	SubmitBlendState(pGraph->GetBlendState());

	m_pCurGraphics = pGraph;

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SubmitWorldMat(const fcyMatrix4& Mat)
{
	m_CurWorldMat = Mat;

	if(FCYFAILED(m_pDev->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&Mat)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SubmitLookatMat(const fcyMatrix4& Mat)
{
	m_CurLookatMat = Mat;

	if(FCYFAILED(m_pDev->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&Mat)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SubmitProjMat(const fcyMatrix4& Mat)
{
	m_CurProjMat = Mat;

	if(FCYFAILED(m_pDev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&Mat)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SubmitBlendState(const f2dBlendState& State)
{
	// 对比状态并设置
	if(m_CurBlendState.AlphaBlendOp != State.AlphaBlendOp)
		m_pDev->SetRenderState(D3DRS_BLENDOPALPHA, State.AlphaBlendOp);
	if(m_CurBlendState.AlphaDestBlend != State.AlphaDestBlend)
		m_pDev->SetRenderState(D3DRS_DESTBLENDALPHA, State.AlphaDestBlend);
	if(m_CurBlendState.AlphaSrcBlend != State.AlphaSrcBlend)
		m_pDev->SetRenderState(D3DRS_SRCBLENDALPHA, State.AlphaSrcBlend);
	if(m_CurBlendState.BlendOp != State.BlendOp)
		m_pDev->SetRenderState(D3DRS_BLENDOP, State.BlendOp);
	if(m_CurBlendState.DestBlend != State.DestBlend)
		m_pDev->SetRenderState(D3DRS_DESTBLEND, State.DestBlend);
	if(m_CurBlendState.SrcBlend != State.SrcBlend)
		m_pDev->SetRenderState(D3DRS_SRCBLEND, State.SrcBlend);

	m_CurBlendState = State;

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SubmitVD(IDirect3DVertexDeclaration9* pVD)
{
	if(m_pCurVertDecl == pVD)
		return FCYERR_OK;
	else
	{
		m_pCurVertDecl = pVD;
		if(FAILED(m_pDev->SetVertexDeclaration(pVD)))
			return FCYERR_INTERNALERR;
		else
			return FCYERR_OK;
	}
}

fResult f2dRenderDeviceImpl::SubmitTextureBlendOP_Color(D3DTEXTUREOP ColorOP)
{
	if(m_CurTexBlendOP_Color == ColorOP)
		return FCYERR_OK;
	else
	{
		if(FAILED(m_pDev->SetTextureStageState(0, D3DTSS_COLOROP, ColorOP)))
			return FCYERR_INTERNALERR;
		else
		{
			m_CurTexBlendOP_Color = ColorOP;
			return FCYERR_OK;
		}
	}
}

IDirect3DVertexDeclaration9* f2dRenderDeviceImpl::RegisterVertexDeclare(f2dVertexElement* pElement, fuInt ElementCount, fuInt& ElementSize)
{
	if(ElementCount == 0)
		return NULL;

	// === Hash检查 ===
	fuInt HashCode = fcyHash::SuperFastHash((fcData)pElement, sizeof(f2dVertexElement) * ElementCount);
	
	for(fuInt i = 0; i<m_VDCache.size(); ++i)
	{
		if(m_VDCache[i].ElementData.size() == ElementCount && m_VDCache[i].Hash == HashCode)
		{
			fBool tHas = true;

			std::vector<f2dVertexElement>& tVec = m_VDCache[i].ElementData;
			for(fuInt j = 0; j<tVec.size(); j++)
			{
				if(memcmp(&tVec[j], &pElement[j], sizeof(f2dVertexElement))!=0)
				{
					tHas = false;
					break;
				}
			}

			/*
				感谢漆黑の刃提出issue
			*/
			if (tHas)
			{
				ElementSize = m_VDCache[i].VertexSize;
				return m_VDCache[i].pVertexDeclare;
			}
		}
	}

	// === 创建条目 ===
	D3DVERTEXELEMENT9* pElementArr = new D3DVERTEXELEMENT9[ ElementCount + 1 ];
	ZeroMemory(pElementArr, sizeof(D3DVERTEXELEMENT9) * (ElementCount + 1));

	// 结尾
	D3DVERTEXELEMENT9 tEnd = D3DDECL_END();
	pElementArr[ ElementCount ] = tEnd;

	fuInt tOffset = 0;

	for(fuInt i = 0; i<ElementCount; ++i)
	{
		pElementArr[i].Stream = 0;
		pElementArr[i].Offset = tOffset;
		pElementArr[i].Type = pElement[i].Type;
		pElementArr[i].Method = D3DDECLMETHOD_DEFAULT;
		pElementArr[i].Usage = pElement[i].Usage;
		pElementArr[i].UsageIndex = pElement[i].UsageIndex;

		switch(pElementArr[i].Type)
		{
		case D3DDECLTYPE_FLOAT1:
			tOffset += sizeof(float);
			break;
		case D3DDECLTYPE_FLOAT2:
			tOffset += sizeof(float) * 2;
			break;
		case D3DDECLTYPE_FLOAT3:
			tOffset += sizeof(float) * 3;
			break;
		case D3DDECLTYPE_FLOAT4:
			tOffset += sizeof(float) * 4;
			break;
		case D3DDECLTYPE_D3DCOLOR:
			tOffset += sizeof(char) * 4;
			break;
		case D3DDECLTYPE_UBYTE4:
			tOffset += sizeof(char) * 4;
			break;
		case D3DDECLTYPE_SHORT2:
			tOffset += sizeof(short) * 2;
			break;
		case D3DDECLTYPE_SHORT4:
			tOffset += sizeof(short) * 4;
			break;
		}
	}

	IDirect3DVertexDeclaration9* pVD = NULL;
	HRESULT tHR = m_pDev->CreateVertexDeclaration(pElementArr, &pVD);
	FCYSAFEDELARR(pElementArr);

	if(FAILED(tHR))
		return NULL;

	VertexDeclareInfo tInfo;
	tInfo.Hash = HashCode;
	tInfo.pVertexDeclare = pVD;
	tInfo.ElementData.resize(ElementCount);
	tInfo.VertexSize = tOffset;
	for(fuInt i = 0; i<ElementCount; ++i)
	{
		tInfo.ElementData[i] = pElement[i];
	}
	m_VDCache.push_back(tInfo);

	ElementSize = tOffset;

	return pVD;
}

F2DAALEVEL f2dRenderDeviceImpl::GetAALevel()
{
	switch(_d3d9SwapChainInfo.MultiSampleType)
	{
	case D3DMULTISAMPLE_2_SAMPLES:
		return F2DAALEVEL_2;
	case D3DMULTISAMPLE_4_SAMPLES:
		return F2DAALEVEL_4;
	case D3DMULTISAMPLE_8_SAMPLES:
		return F2DAALEVEL_8;
	case D3DMULTISAMPLE_16_SAMPLES:
		return F2DAALEVEL_16;
	}
	return F2DAALEVEL_NONE;
}

fBool f2dRenderDeviceImpl::CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed)
{
	DWORD Quality = 0;
	HRESULT tHr;

	switch(AALevel)
	{
	case F2DAALEVEL_NONE:
		return true;
	case F2DAALEVEL_2:
		tHr = m_pD3D9->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, Windowed, D3DMULTISAMPLE_2_SAMPLES, &Quality);
		break;
	case F2DAALEVEL_4:
		tHr = m_pD3D9->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, Windowed, D3DMULTISAMPLE_4_SAMPLES, &Quality);
		break;
	case F2DAALEVEL_8:
		tHr = m_pD3D9->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, Windowed, D3DMULTISAMPLE_8_SAMPLES, &Quality);
		break;
	case F2DAALEVEL_16:
		tHr = m_pD3D9->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, Windowed, D3DMULTISAMPLE_16_SAMPLES, &Quality);
		break;
	default:
		return false;
	}

	if(FAILED(tHr))
		return false;
	else
		return true;
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

fResult f2dRenderDeviceImpl::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel)
{
	// 检查参数
	if (Width < 1 || Height < 1)
		return FCYERR_INVAILDPARAM;
	
	// 备份交换链配置
	D3DPRESENT_PARAMETERS D3DPP = _d3d9SwapChainInfo;
	D3DDISPLAYMODEEX D3DPPEX = _d3d9FullScreenSwapChainInfo;
	
	// 有没有找到合适的全屏模式？
	if (!_findMatchDisplayMode(_d3d9Ex.Get(), _d3d9FullScreenSwapChainInfo, Width, Height))
	{
		if (!Windowed) // 想全屏，但是没匹配的模式
		{
			m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::SetBufferSize",
				"Cannot find a valid full-screen display mode (%ux%u).", Width, Height));
			return FCYERR_INVAILDPARAM;
		}
	}
	
	// 准备参数
	auto& winfo = _d3d9SwapChainInfo; {
		winfo.BackBufferWidth      = Width;
		winfo.BackBufferHeight     = Height;
		winfo.BackBufferCount      = 1;
		winfo.SwapEffect           = D3DSWAPEFFECT_DISCARD;
		if (Windowed && _d3d9SupportFlip && FlipModel)
		{
			//winfo.BackBufferCount  = 2;
			winfo.SwapEffect       = D3DSWAPEFFECT_FLIPEX; // 窗口模式下，且支持FLIPEX交换链模型，就可以开启
		}
		winfo.BackBufferFormat     = Windowed ? D3DFMT_UNKNOWN : _d3d9FullScreenSwapChainInfo.Format; // 窗口模式下让d3d9决定画面格式，否则必须指定格式
		winfo.Windowed             = Windowed;
		winfo.FullScreen_RefreshRateInHz = Windowed ? 0 : _d3d9FullScreenSwapChainInfo.RefreshRate; // 窗口模式下让d3d9决定刷新率，否则必须指定刷新率
		winfo.PresentationInterval = VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	};
	
	// 首先，通知一些资源需要释放
	dispatchRenderSizeDependentResourcesDestroy();
	
	HRESULT hr = doReset();
	if (hr == D3D_OK)
	{
		// 成功，更新状态
		
		m_ScissorRect.left = 0;
		m_ScissorRect.top = 0;
		m_ScissorRect.right = GetBufferWidth();
		m_ScissorRect.bottom = GetBufferHeight();

		m_ViewPort.X = 0;
		m_ViewPort.Y = 0;
		m_ViewPort.Width = GetBufferWidth();
		m_ViewPort.Height = GetBufferHeight();
		m_ViewPort.MinZ = 0.0f;
		m_ViewPort.MaxZ = 1.0f;
		
		// 恢复资源
		dispatchRenderSizeDependentResourcesCreate();
	}
	else
	{
		// 我们假设上一个交换链配置是有效的
		_d3d9SwapChainInfo = D3DPP;
		_d3d9FullScreenSwapChainInfo = D3DPPEX;
		// 设备丢失，广播设备丢失事件
		m_bDevLost = true; // 标记为设备丢失状态
		int tObjCount = sendDevLostMsg();
		char tBuffer[256] = {};
		snprintf(tBuffer, 255, "Detected device lost. ( %d Object(s) losted. )", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::SetBufferSize", tBuffer));
	}
	
	return hr == D3D_OK ? FCYERR_OK : FCYERR_INTERNALERR;
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

fResult f2dRenderDeviceImpl::CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dGraphics3DImpl(this, (f2dEffectImpl*)pDefaultEffect);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dEffectImpl(this, pStream, bAutoState);
	}
	catch(const fcyException& e)
	{
		m_pEngine->ThrowException(e);

		return FCYERR_INTERNALERR;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;
	*pOut = NULL;

	try
	{
		*pOut = new f2dMeshDataImpl(this, pVertElement, ElementCount, VertCount, IndexCount, Int32Index);
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
	if (v != m_bZBufferEnabled)
	{
		if (m_pCurGraphics && m_pCurGraphics->IsInRender())
			m_pCurGraphics->Flush();
		if (FAILED(m_pDev->SetRenderState(D3DRS_ZENABLE, v ? TRUE : FALSE)))
			return FCYERR_INTERNALERR;
		m_bZBufferEnabled = v;
		/*
		if (v == TRUE){
			void TestDraw();
			TestDraw();
		}*/
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::UpdateScreenToWindow(fcyColor KeyColor, fByte Alpha)
{
	fuInt tBackWidth = GetBufferWidth();
	fuInt tBackHeight = GetBufferHeight();

	// === 检查窗口大小是否匹配 ===
	if(m_DC.GetSafeHdc() == NULL || m_pWinSurface == NULL || (fuInt)m_DC.GetWidth() != tBackWidth || (fuInt)m_DC.GetHeight() != tBackHeight)
	{
		m_DC.Delete();
		m_DC.Create(tBackWidth, tBackHeight);

		// 创建离屏页面
		FCYSAFEKILL(m_pWinSurface);
		if(FAILED(m_pDev->CreateOffscreenPlainSurface(tBackWidth, tBackHeight, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_pWinSurface, NULL)))
			return FCYERR_INTERNALERR;
	}

	// === 准备参数 ===
	RECT tWinRect;
	::GetWindowRect(m_hWnd, &tWinRect);
	HDC tWinDC = GetWindowDC(m_hWnd);
	// 窗口位置
	POINT ptWinPos = { tWinRect.left, tWinRect.top };
	// DC数据起始位置
	POINT ptSrc = { 0, 0 };
	// 窗口大小
	SIZE szWin = { tWinRect.right - tWinRect.left, tWinRect.bottom - tWinRect.top };
	// DC表面大小
	RECT tDCSurface = { 0, 0, (LONG)tBackWidth, (LONG)tBackHeight };
	BLENDFUNCTION stBlend = { AC_SRC_OVER, 0, Alpha, AC_SRC_ALPHA };
	
	// === 拷贝BackSurface ===
	D3DLOCKED_RECT tD3DLockRect;
	IDirect3DSurface9* pOrgSurface = NULL;

	if(FAILED(m_pDev->GetRenderTarget(0, &pOrgSurface)))
		return FCYERR_INTERNALERR;
	if(FAILED(m_pDev->GetRenderTargetData(pOrgSurface, m_pWinSurface)))
	{
		FCYSAFEKILL(pOrgSurface);
		return FCYERR_INTERNALERR;
	}

	FCYSAFEKILL(pOrgSurface);

	// === 拷贝到GDI ===
	if(FAILED(m_pWinSurface->LockRect(&tD3DLockRect, &tDCSurface, D3DLOCK_READONLY)))
		return FCYERR_INTERNALERR;
	fuInt y = 0;
	fuInt x = 0;
	fcyColor* tDest = (fcyColor*)m_DC.GetBits();
	fcyColor* tSrc = (fcyColor*)tD3DLockRect.pBits;
	while(y < tBackHeight)
	{
		*tDest = tSrc[x];
		tDest++; x++;
		if(x >= tBackWidth)
		{
			tSrc = (fcyColor*)((fByte*)tSrc + tD3DLockRect.Pitch);
			x = 0;
			y++;
		}
	}
	m_pWinSurface->UnlockRect();

	if(FALSE == UpdateLayeredWindow(m_hWnd, tWinDC, &ptWinPos, &szWin, m_DC.GetSafeHdc(), &ptSrc, 0, &stBlend, ULW_ALPHA))
		return FCYERR_INTERNALERR;
	
	ReleaseDC(m_hWnd, tWinDC);

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor) {
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();
	
	D3DTEXTUREADDRESS d3daddress;
	switch (address) {
	case F2DTEXTUREADDRESS_WRAP:
		d3daddress = D3DTADDRESS_WRAP;
		break;
	case F2DTEXTUREADDRESS_MIRROR:
		d3daddress = D3DTADDRESS_MIRROR;
		break;
	case F2DTEXTUREADDRESS_CLAMP:
		d3daddress = D3DTADDRESS_CLAMP;
		break;
	case F2DTEXTUREADDRESS_BORDER:
		d3daddress = D3DTADDRESS_BORDER;
		break;
	case F2DTEXTUREADDRESS_MIRRORONCE:
		d3daddress = D3DTADDRESS_MIRRORONCE;
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}
	if (FAILED(m_pDev->SetSamplerState(0, D3DSAMP_ADDRESSU, d3daddress)))
		return FCYERR_INTERNALERR;
	if (FAILED(m_pDev->SetSamplerState(0, D3DSAMP_ADDRESSV, d3daddress)))
		return FCYERR_INTERNALERR;
	if (FAILED(m_pDev->SetSamplerState(0, D3DSAMP_BORDERCOLOR, borderColor.argb)))
		return FCYERR_INTERNALERR;
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SetTextureFilter(F2DTEXFILTERTYPE filter) {
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	D3DTEXTUREFILTERTYPE d3dfilter;
	switch (filter)
	{
	case F2DTEXFILTER_POINT:
		d3dfilter = D3DTEXF_POINT;
		break;
	case F2DTEXFILTER_LINEAR:
		d3dfilter = D3DTEXF_LINEAR;
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}
	if (FAILED(m_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, d3dfilter)))
		return FCYERR_INTERNALERR;
	if (FAILED(m_pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, d3dfilter)))
		return FCYERR_INTERNALERR;
	if (FAILED(m_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, d3dfilter)))
		return FCYERR_INTERNALERR;
	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

#include "ScreenGrab9.h"

fResult f2dRenderDeviceImpl::SaveScreen(fcStrW path)
{
	if(m_bDevLost)
		return FCYERR_ILLEGAL;
	if(!path)
		return FCYERR_INVAILDPARAM;
	
	// 获取交换链后备缓冲区
	Microsoft::WRL::ComPtr<IDirect3DSurface9> backbuffer;
	if(FAILED(m_pDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, backbuffer.GetAddressOf())))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 获取尺寸、纹理格式
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
	if(FAILED(backbuffer->GetDesc(&desc)))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 创建Stage纹理，用来从显存下载纹理内容到内存
	Microsoft::WRL::ComPtr<IDirect3DTexture9> rendertarget;
	if(FAILED(m_pDev->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_DYNAMIC, desc.Format, D3DPOOL_SYSTEMMEM,
		rendertarget.GetAddressOf(), NULL)))
	{
		return FCYERR_INTERNALERR;
	}
	Microsoft::WRL::ComPtr<IDirect3DSurface9> surface;
	if(FAILED(rendertarget->GetSurfaceLevel(0, surface.GetAddressOf())))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 下崽
	if(FAILED(m_pDev->GetRenderTargetData(backbuffer.Get(), surface.Get())))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 可以保存了
	HRESULT tHR = DirectX::SaveWICTextureToFile(surface.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if(FAILED(tHR))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDeviceImpl::SaveScreen", "DirectX::SaveWICTextureToFile Failed.", tHR));
		return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::SaveTexture(fcStrW path, f2dTexture2D* pTex)
{
	if(m_bDevLost)
		return FCYERR_ILLEGAL;
	if(!path || !pTex)
		return FCYERR_INVAILDPARAM;
	if(!pTex->IsRenderTarget())
		return FCYERR_INVAILDPARAM;
	
	// 获取纹理
	Microsoft::WRL::ComPtr<IDirect3DSurface9> backbuffer;
	f2dTexture2DRenderTarget* pRTex = dynamic_cast<f2dTexture2DRenderTarget*>(pTex);
	*backbuffer.GetAddressOf() = pRTex->GetSurface();
	if (!backbuffer.Get())
		return FCYERR_INTERNALERR;
	backbuffer->AddRef(); // 这里用了智障指针，得手动加一个ref
	
	// 获取尺寸、纹理格式
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));
	if(FAILED(backbuffer->GetDesc(&desc)))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 创建Stage纹理，用来从显存下载纹理内容到内存
	Microsoft::WRL::ComPtr<IDirect3DTexture9> rendertarget;
	if(FAILED(m_pDev->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_DYNAMIC, desc.Format, D3DPOOL_SYSTEMMEM,
		rendertarget.GetAddressOf(), NULL)))
	{
		return FCYERR_INTERNALERR;
	}
	Microsoft::WRL::ComPtr<IDirect3DSurface9> surface;
	if(FAILED(rendertarget->GetSurfaceLevel(0, surface.GetAddressOf())))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 下崽
	if(FAILED(m_pDev->GetRenderTargetData(backbuffer.Get(), surface.Get())))
	{
		return FCYERR_INTERNALERR;
	}
	
	// 现在可以保存了
	HRESULT tHR = DirectX::SaveWICTextureToFile(surface.Get(), GUID_ContainerFormatJpeg, path, &GUID_WICPixelFormat24bppBGR);
	if(FAILED(tHR))
	{
		m_pEngine->ThrowException(fcyWin32COMException("f2dRenderDeviceImpl::SaveTexture", "DirectX::SaveWICTextureToFile Failed.", tHR));
		return FCYERR_INTERNALERR;
	}
	
	return FCYERR_OK;
}
