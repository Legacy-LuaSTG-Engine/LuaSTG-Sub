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
#include <d3d9.h>

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

f2dRenderDeviceImpl::f2dRenderDeviceImpl(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)
: m_pEngine(pEngine), m_pD3D9(NULL), m_pDev(NULL), m_hWnd((HWND)pEngine->GetMainWindow()->GetHandle()), 
	m_pSyncTestObj(NULL), m_bDevLost(false), m_pBackBuffer(NULL), m_pBackDepthBuffer(NULL), m_pCurGraphics(NULL),
	m_ListenerList(NULL), m_pWinSurface(NULL), m_pCurBackBuffer(NULL), m_pCurBackDepthBuffer(NULL),
	m_pCurVertDecl(NULL), m_CreateThreadID(GetCurrentThreadId()), m_bZBufferEnabled(true)
{
	ZeroMemory(&m_D3Dpp,sizeof(m_D3Dpp));
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = BackBufferWidth;
	m_ScissorRect.bottom = BackBufferHeight;

	HRESULT tHR;

	// --- 创建D3D9 ---
	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_pD3D9)
		throw fcyException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "Direct3DCreate9 Failed.");
	
	// --- 创建设备 ---
	// 填充属性
	m_D3Dpp.BackBufferCount = 1;                      // 后台缓冲页面个数
	m_D3Dpp.BackBufferWidth = BackBufferWidth;        // 缓冲页面宽度
	m_D3Dpp.BackBufferHeight = BackBufferHeight;      // 缓冲页面高度
	m_D3Dpp.BackBufferFormat = D3DFMT_A8R8G8B8;       // 页面格式
	m_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24S8;    // 模板和Z缓冲格式
	m_D3Dpp.EnableAutoDepthStencil = true;            // 开启自动创建模板和Z缓冲
	m_D3Dpp.Windowed = Windowed;                      // 是否窗口
	m_D3Dpp.PresentationInterval = VSync? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;	// 立即交换
	m_D3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_D3Dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if(AALevel != F2DAALEVEL_NONE && CheckMultiSample(AALevel, Windowed))
	{
		// 开启抗锯齿
		m_D3Dpp.MultiSampleQuality = 0;

		// 开启抗锯齿
		switch(AALevel)
		{
		case F2DAALEVEL_2:
			m_D3Dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
			break;
		case F2DAALEVEL_4:
			m_D3Dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
			break;
		case F2DAALEVEL_8:
			m_D3Dpp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
			break;
		case F2DAALEVEL_16:
			m_D3Dpp.MultiSampleType = D3DMULTISAMPLE_16_SAMPLES;
			break;
		default:
			m_D3Dpp.MultiSampleQuality = 0;
			break;
		}
	}

	tHR = m_pD3D9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		m_hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, // 启动多线程
		&m_D3Dpp,
		&m_pDev
		);

	if (FAILED(tHR))
	{
		tHR = m_pD3D9->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, // 启动多线程
			&m_D3Dpp,
			&m_pDev
		);
	}

	if(FAILED(tHR))
	{
		FCYSAFEKILL(m_pD3D9);
		throw fcyWin32COMException("f2dRenderDeviceImpl::f2dRenderDeviceImpl", "IDirect3D9::CreateDevice Failed.", tHR);
	}
	else
		m_pSyncTestObj = new DeviceSyncTest(m_pDev);

	// --- 获取设备参数 ---
	D3DADAPTER_IDENTIFIER9 tIdentify;
	m_pD3D9->GetAdapterIdentifier(0, 0, &tIdentify);
	m_DevName = tIdentify.Description;

	// --- 初始化渲染状态 ---
	m_ViewPort.Width = GetBufferWidth();
	m_ViewPort.Height = GetBufferHeight();
	m_ViewPort.MaxZ = 1.0f;
	m_ViewPort.MinZ = 0.0f;
	m_ViewPort.X = 0;
	m_ViewPort.Y = 0;

	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = GetBufferWidth();
	m_ScissorRect.bottom = GetBufferHeight();
	
	m_CurBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
	m_CurBlendState.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
	m_CurBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
	m_CurBlendState.AlphaBlendOp = F2DBLENDOPERATOR_ADD;
	m_CurBlendState.AlphaSrcBlend = F2DBLENDFACTOR_SRCALPHA;
	m_CurBlendState.AlphaDestBlend = F2DBLENDFACTOR_INVSRCALPHA;

	m_CurTexBlendOP_Color = D3DTOP_ADD;

	m_CurWorldMat = fcyMatrix4::GetIdentity();
	m_CurLookatMat = fcyMatrix4::GetIdentity();
	m_CurProjMat = fcyMatrix4::GetIdentity();

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
	ListenerNode* pListener = m_ListenerList;
	while(pListener)
	{
		ListenerNode* p = pListener;

		pListener = pListener->pNext;

		{
			// 报告可能的对象泄漏
			char tTextBuffer[256];
			sprintf_s(tTextBuffer, "Unrelease listener object at %p", p);
#ifdef _DEBUG
			fcyDebug::Trace("[ @ f2dRenderDeviceImpl::~f2dRenderDeviceImpl ] %s\n", tTextBuffer);
#endif
			m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::~f2dRenderDeviceImpl", tTextBuffer));
		}

		delete p;
	}
	
	// 释放顶点声明
	m_VDCache.clear();

	FCYSAFEDEL(m_pSyncTestObj);

	// 释放DX组件
	FCYSAFEKILL(m_pWinSurface);
	FCYSAFEKILL(m_pDev);
	FCYSAFEKILL(m_pD3D9);
}

HRESULT f2dRenderDeviceImpl::doReset(D3DPRESENT_PARAMETERS* pD3DPP)
{
	struct _Work : public fcyRefObjImpl<f2dMainThreadDelegate>
	{
		IDirect3DDevice9* pDev;
		D3DPRESENT_PARAMETERS* D3DPP;
		HRESULT HR;
		
		void Excute() { HR = pDev->Reset(D3DPP); }

		_Work(IDirect3DDevice9* p, D3DPRESENT_PARAMETERS* pp)
			: pDev(p), D3DPP(pp), HR(D3DERR_DEVICELOST) {}
	};

	if(GetCurrentThreadId() != m_CreateThreadID)
	{
		HRESULT tHR;
		_Work* tWork = new _Work(m_pDev, pD3DPP);

		m_pEngine->InvokeDelegateAndWait(tWork);

		tHR = tWork->HR;
		FCYSAFEKILL(tWork);

		return tHR;
	}
	else
	{
		return m_pDev->Reset(pD3DPP);
	}
}

HRESULT f2dRenderDeviceImpl::doTestCooperativeLevel()
{
	if(GetCurrentThreadId() != m_CreateThreadID)
	{
		m_pSyncTestObj->Reset();
		m_pEngine->InvokeDelegateAndWait(m_pSyncTestObj);
		return m_pSyncTestObj->GetResult();
	}
	else
	{
		return m_pDev->TestCooperativeLevel();
	}
}

void f2dRenderDeviceImpl::initState()
{
	// --- 初始化视口 ---
	m_pDev->SetViewport(&m_ViewPort);

	// --- 设置默认渲染状态 ---
	m_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);   // 设置反面剔除
	m_pDev->SetRenderState(D3DRS_LIGHTING, FALSE);    // 关闭光照
	m_pDev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);  // 打开矩形裁剪功能

	// --- 设置ZBUFFER ---
	m_pDev->SetRenderState(D3DRS_ZENABLE, m_bZBufferEnabled);
	m_pDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_pDev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	// --- 设置默认混合状态 ---
	m_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);  // 启动Alpha混合
	m_pDev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

	m_pDev->SetRenderState(D3DRS_BLENDOP, m_CurBlendState.BlendOp);
	m_pDev->SetRenderState(D3DRS_SRCBLEND, m_CurBlendState.SrcBlend);
	m_pDev->SetRenderState(D3DRS_DESTBLEND, m_CurBlendState.DestBlend);
	m_pDev->SetRenderState(D3DRS_BLENDOPALPHA, m_CurBlendState.AlphaBlendOp);
	m_pDev->SetRenderState(D3DRS_SRCBLENDALPHA, m_CurBlendState.AlphaSrcBlend);
	m_pDev->SetRenderState(D3DRS_DESTBLENDALPHA, m_CurBlendState.AlphaDestBlend);

	// --- 纹理混合参数默认值 ---
	m_pDev->SetTextureStageState(0, D3DTSS_COLOROP,  m_CurTexBlendOP_Color);    
	//m_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	//m_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	m_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// --- 手动premul需要把texture放在第一个乘以texture的alpha
	m_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,  D3DTOP_MODULATE);
	m_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	m_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

	// --- 纹理寻址 ---
	m_pDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_pDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// --- 设置采样器 ---
	// 三线性
	m_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	m_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	// --- 其他功能 ---
	// 设置矩形裁剪
	m_pDev->SetScissorRect(&m_ScissorRect);

	// 设置默认矩阵
	m_pDev->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m_CurWorldMat);
	m_pDev->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_CurLookatMat);
	m_pDev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_CurProjMat);

	//void Test(LPDIRECT3DDEVICE9 pd3d);
	//Test(m_pDev);

}

int f2dRenderDeviceImpl::sendDevLostMsg()
{
	int tRet = 0;

	m_pCurVertDecl = NULL;

	// 释放可能的对象
	FCYSAFEKILL(m_pCurBackBuffer);
	FCYSAFEKILL(m_pCurBackDepthBuffer);
	FCYSAFEKILL(m_pBackBuffer);
	FCYSAFEKILL(m_pBackDepthBuffer);
	
	// 发送丢失消息
	ListenerNode* pListener = m_ListenerList;
	while(pListener)
	{
		pListener->pListener->OnRenderDeviceLost();
		pListener = pListener->pNext;
		
		tRet++;
	}

	return tRet;
}

int f2dRenderDeviceImpl::sendDevResetMsg()
{
	int tRet = 0;

	// 发送重置消息
	ListenerNode* pListener = m_ListenerList;
	while(pListener)
	{
		pListener->pListener->OnRenderDeviceReset();
		pListener = pListener->pNext;

		tRet++;
	}

	return tRet;
}

fResult f2dRenderDeviceImpl::SyncDevice()
{
	HRESULT tHR = doTestCooperativeLevel();	// 设备协作测试
	
	if(m_bDevLost) // 设备已经丢失
	{	
		if(tHR == D3DERR_DEVICENOTRESET) // 尚未重置
		{
			tHR = doReset(&m_D3Dpp);

			if(SUCCEEDED(tHR)) // 重置成功
			{
				m_bDevLost = false;

				initState(); // 重新初始化状态

				int tObjCount = sendDevResetMsg(); // 通知非托管组件恢复工作

				fChar tBuffer[256];
				sprintf_s(tBuffer, "Device reseted. ( %d Object(s) rested. )", tObjCount);
				m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::SyncDevice", tBuffer));
			}
		}
	}
	else if(tHR == D3DERR_DEVICELOST || tHR == D3DERR_DEVICENOTRESET) 	// 设备尚未丢失，检查是否丢失
	{
		m_bDevLost = true; // 标记设备丢失
		
		int tObjCount = sendDevLostMsg();

		fChar tBuffer[256];
		sprintf_s(tBuffer, "Detected device lost. ( %d Object(s) losted. )", tObjCount);
		m_pEngine->ThrowException(fcyException("f2dRenderDeviceImpl::SyncDevice", tBuffer));
	}

	return (m_bDevLost ? FCYERR_INTERNALERR : FCYERR_OK);
}

fResult f2dRenderDeviceImpl::Present()
{
	// 还原当前的后台缓冲区
	if(m_pBackBuffer)
	{
		m_pDev->SetRenderTarget(0, m_pBackBuffer);

		// 此时vp已发生变化，跟踪新的vp
		m_pDev->GetViewport(&m_ViewPort);

		FCYSAFEKILL(m_pBackBuffer);
		FCYSAFEKILL(m_pCurBackBuffer);
	}
	if(m_pBackDepthBuffer)
	{
		m_pDev->SetDepthStencilSurface(m_pBackDepthBuffer);
		FCYSAFEKILL(m_pBackDepthBuffer);
		FCYSAFEKILL(m_pCurBackDepthBuffer);
	}

	if(FAILED(m_pDev->Present(0,0,0,0)))
		return FCYERR_INTERNALERR;
	else
		return FCYERR_OK;
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
	switch(m_D3Dpp.MultiSampleType)
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
	return m_pD3D9->GetAdapterModeCount(0, D3DFMT_X8R8G8B8);
}

fcyVec2 f2dRenderDeviceImpl::EnumSupportResolution(fuInt Index)
{
	fcyVec2 tRet;
	D3DDISPLAYMODE tMode;

	if(SUCCEEDED(m_pD3D9->EnumAdapterModes(0, D3DFMT_X8R8G8B8, Index, &tMode)))
	{
		tRet.x = (float)tMode.Width;
		tRet.y = (float)tMode.Height;
	}
	
	return tRet;
}

fResult f2dRenderDeviceImpl::SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)
{
	// 准备参数
	D3DPRESENT_PARAMETERS tD3Dpp;
	ZeroMemory(&tD3Dpp, sizeof(D3DPRESENT_PARAMETERS));

	tD3Dpp.BackBufferCount = 1;                      // 后台缓冲页面个数
	tD3Dpp.BackBufferWidth = Width;	                 // 缓冲页面宽度
	tD3Dpp.BackBufferHeight = Height;                // 缓冲页面高度
	tD3Dpp.BackBufferFormat = D3DFMT_A8R8G8B8;       // 页面格式
	tD3Dpp.AutoDepthStencilFormat = D3DFMT_D24S8;    // 模板和Z缓冲格式
	tD3Dpp.EnableAutoDepthStencil = true;            // 开启自动创建模板和Z缓冲
	tD3Dpp.Windowed = Windowed;                      // 是否窗口
	tD3Dpp.PresentationInterval = VSync? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;	// 立即交换
	tD3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	tD3Dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if(AALevel != F2DAALEVEL_NONE)
	{
		tD3Dpp.MultiSampleQuality = 0;

		// 开启抗锯齿
		switch(AALevel)
		{
		case F2DAALEVEL_2:
			tD3Dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
			break;
		case F2DAALEVEL_4:
			tD3Dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
			break;
		case F2DAALEVEL_8:
			tD3Dpp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
			break;
		case F2DAALEVEL_16:
			tD3Dpp.MultiSampleType = D3DMULTISAMPLE_16_SAMPLES;
			break;
		default:
			tD3Dpp.MultiSampleQuality = 0;
			break;
		}
	}

	// 销毁所有未托管对象
	m_bDevLost = true;
	sendDevLostMsg();

	// 重设渲染系统
	HRESULT tHR = doReset(&tD3Dpp);
	if(FAILED(tHR))
	{
		// 设置失败，保留为丢失状态，等待循环时恢复上次状态。
		return FCYERR_INVAILDPARAM;
	}
	else
	{
		// 成功，恢复所有状态
		m_bDevLost = false;
		m_D3Dpp = tD3Dpp;

		m_ScissorRect.left = 0;
		m_ScissorRect.top = 0;
		m_ScissorRect.right = GetBufferWidth();
		m_ScissorRect.bottom = GetBufferHeight();

		m_ViewPort.Width = GetBufferWidth();
		m_ViewPort.Height = GetBufferHeight();
		m_ViewPort.MaxZ = 1.0f;
		m_ViewPort.MinZ = 0.0f;
		m_ViewPort.X = 0;
		m_ViewPort.Y = 0;

		initState();

		sendDevResetMsg();

		return FCYERR_OK;
	}
}

fResult f2dRenderDeviceImpl::AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority)
{
	if(Listener == NULL)
		return FCYERR_INVAILDPARAM;

	ListenerNode* pNew = new ListenerNode();
	pNew->pListener = Listener;
	pNew->Priority = Priority;
	pNew->pNext = m_ListenerList;

	m_ListenerList = pNew;

	// 对优先级进行插入排序
	ListenerNode* p = m_ListenerList;
	while(p->pNext && p->Priority > p->pNext->Priority)
	{
		ListenerNode* t = p->pNext;

		std::swap(p->pListener, t->pListener);
		std::swap(p->Priority, t->Priority);

		p = t;
	}

	return FCYERR_OK;
}

fResult f2dRenderDeviceImpl::RemoveListener(f2dRenderDeviceEventListener* Listener)
{
	if(m_ListenerList)
	{
		if(m_ListenerList->pListener == Listener)
		{
			ListenerNode* p = m_ListenerList->pNext;
			delete m_ListenerList;
			m_ListenerList = p;
		}
		else
		{
			ListenerNode* p = m_ListenerList;
			while(p)
			{
				ListenerNode* pNext = p->pNext;

				if(pNext && pNext->pListener == Listener)
				{
					p->pNext = pNext->pNext;
					delete pNext;
					return FCYERR_OK;
				}

				p = pNext;
			}
		}
	}

	return FCYERR_OBJNOTEXSIT;
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
	fcyRect tRet((float)m_ScissorRect.left, (float)m_ScissorRect.top, (float)m_ScissorRect.right, (float)m_ScissorRect.bottom);
	return tRet;
}

fResult f2dRenderDeviceImpl::SetScissorRect(const fcyRect& pRect)
{
	if (m_pCurGraphics && m_pCurGraphics->IsInRender())
		m_pCurGraphics->Flush();

	m_ScissorRect.left = (int)pRect.a.x;
	m_ScissorRect.top = (int)pRect.a.y;
	m_ScissorRect.right = (int)pRect.b.x;
	m_ScissorRect.bottom = (int)pRect.b.y;

	if(FAILED(m_pDev->SetScissorRect(&m_ScissorRect)))
		return FCYERR_INTERNALERR;
	else
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

	if (!vp.Intersect(fcyRect(0, 0, (float)GetBufferWidth(), (float)GetBufferHeight()), &vp))
		return FCYERR_ILLEGAL;

	DWORD tNewX = (DWORD)vp.a.x;
	DWORD tNewY = (DWORD)vp.a.y;
	DWORD tNewW = (DWORD)vp.GetWidth();
	DWORD tNewH = (DWORD)vp.GetHeight();

	if (tNewX != m_ViewPort.X || tNewY != m_ViewPort.Y || tNewW != m_ViewPort.Width || tNewH != m_ViewPort.Height)
	{
		m_ViewPort.X = tNewX;
		m_ViewPort.Y = tNewY;
		m_ViewPort.Width = tNewW;
		m_ViewPort.Height = tNewH;

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

#include <wrl.h>
#include <wincodec.h>
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
