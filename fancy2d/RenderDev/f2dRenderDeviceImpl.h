////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDeviceImpl.h
/// @brief fancy2D渲染设备接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "RenderDev/f2dRenderDeviceAPI.h"

#include "Engine/f2dWindowImpl.h"
#include "f2dEngine.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染设备实现
////////////////////////////////////////////////////////////////////////////////
class f2dRenderDeviceImpl :
	public fcyRefObjImpl<f2dRenderDevice>
{
private:
	struct ListenerNode
	{
		f2dRenderDeviceEventListener* pListener;
		fInt Priority;   // 优先级

		ListenerNode* pNext;
	};

	// 解决从非主线程调用TestCooperativeLevel的问题
	class DeviceSyncTest
		: public f2dMainThreadDelegate
	{
	private:
		IDirect3DDevice9* m_pDev;
		HRESULT m_HR;
	public:
		void AddRef() { /* do nothing */ }
		void Release() { /* do nothing */ }
		void Excute() { m_HR = m_pDev->TestCooperativeLevel(); }
		void Reset() { m_HR = S_OK; }
		HRESULT GetResult()const { return m_HR; }
	public:
		DeviceSyncTest(IDirect3DDevice9* p)
			: m_pDev(p), m_HR(S_OK) {}
	};

	struct VertexDeclareInfo
	{
		fuInt Hash;
		std::vector<f2dVertexElement> ElementData;
		IDirect3DVertexDeclaration9* pVertexDeclare;
		fuInt VertexSize;

		VertexDeclareInfo();
		VertexDeclareInfo(const VertexDeclareInfo& Org);
		~VertexDeclareInfo();
	protected:
		VertexDeclareInfo& operator=(const VertexDeclareInfo& Right);
	};
private:
	f2dEngineImpl* m_pEngine;
	DWORD m_CreateThreadID;

	// API入口
	f2dRenderDeviceAPI m_API;

	// DirectX组件
	IDirect3D9* m_pD3D9;
	IDirect3DDevice9* m_pDev;
	D3DPRESENT_PARAMETERS m_D3Dpp;
	D3DVIEWPORT9 m_ViewPort;
	std::string m_DevName;

	// 跨线程事件
	DeviceSyncTest* m_pSyncTestObj;
	
	// 监听器链表
	ListenerNode* m_ListenerList;

	// 顶点声明
	std::vector<VertexDeclareInfo> m_VDCache;

	// 设备状态
	bool m_bDevLost;                              // 设备丢失标志
	bool m_bZBufferEnabled;                       // 是否开启Z-缓冲区
	IDirect3DSurface9* m_pBackBuffer;             // 后备渲染目标
	IDirect3DSurface9* m_pBackDepthBuffer;        // 后备深度模板缓冲
	RECT m_ScissorRect;                           // 裁剪矩形
	
	f2dTexture2D* m_pCurBackBuffer;               // 记录当前的后备缓冲区
	f2dDepthStencilSurface* m_pCurBackDepthBuffer;// 记录当前的后备深度缓冲区

	f2dGraphics* m_pCurGraphics;   // 当前的绘图对象，只记录指针
	fcyMatrix4 m_CurWorldMat;      // 当前世界矩阵
	fcyMatrix4 m_CurLookatMat;     // 当前观察矩阵
	fcyMatrix4 m_CurProjMat;       // 当前投影矩阵
	f2dBlendState m_CurBlendState; // 当前混合状态

	IDirect3DVertexDeclaration9* m_pCurVertDecl; // 当前的顶点声明
	D3DTEXTUREOP m_CurTexBlendOP_Color;          // 当前的纹理混合运算符

	// Window
	HWND m_hWnd;
	f2dWindowDC m_DC;
	IDirect3DSurface9* m_pWinSurface;
private:
	HRESULT doReset(D3DPRESENT_PARAMETERS* pD3DPP);  // 保证在主线程执行
	HRESULT doTestCooperativeLevel();
	void initState();         // 初始化状态
	int sendDevLostMsg();     // 发送设备丢失事件, 返回对象数目
	int sendDevResetMsg();    // 发送设备重置事件
public: // 内部函数
	f2dRenderDeviceAPI& GetAPI() { return m_API; }   // 返回API对象
	f2dEngineImpl* GetEngine() { return m_pEngine; } // 返回引擎对象
	fResult SyncDevice();                           // 协作测试，完成设备丢失处理
	fResult Present();                              // 呈现

	// 立即递交状态
	f2dGraphics* QueryCurGraphics() { return m_pCurGraphics; }
	fResult SubmitCurGraphics(f2dGraphics* pGraph, bool bDirty);  // 同步Graphics状态到设备
	fResult SubmitWorldMat(const fcyMatrix4& Mat);                // 立即提交世界变换矩阵
	fResult SubmitLookatMat(const fcyMatrix4& Mat);               // 立即提交观察矩阵
	fResult SubmitProjMat(const fcyMatrix4& Mat);                 // 立即提交投影矩阵
	fResult SubmitBlendState(const f2dBlendState& State);         // 立即提交混合状态

	fResult SubmitVD(IDirect3DVertexDeclaration9* pVD);        // 立即递交顶点声明
	fResult SubmitTextureBlendOP_Color(D3DTEXTUREOP ColorOP);  // 立即递交纹理混合声明

	// 注册顶点声明
	IDirect3DVertexDeclaration9* RegisterVertexDeclare(f2dVertexElement* pElement, fuInt ElementCount, fuInt& ElementSize);
public: // 接口实现
	void* GetHandle() { return m_pDev; }
	fcStr GetDeviceName() { return m_DevName.c_str(); }
	fuInt GetBufferWidth() { return m_D3Dpp.BackBufferWidth; }
	fuInt GetBufferHeight() { return m_D3Dpp.BackBufferHeight; }
	fBool IsWindowed() { return m_D3Dpp.Windowed == TRUE ? 1 : 0; }
	F2DAALEVEL GetAALevel();
	fBool CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed);
	fuInt GetSupportResolutionCount();
	fcyVec2 EnumSupportResolution(fuInt Index);
	fResult SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, F2DAALEVEL AALevel);

	fResult AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority=0);
	fResult RemoveListener(f2dRenderDeviceEventListener* Listener);

	fResult CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut);
	fResult CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize , f2dTexture2D** pOut);
	fResult CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut);
	fResult CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut);
	fResult CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut);
	fResult CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut);
	fResult CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut);

	fResult Clear(const fcyColor& BackBufferColor = 0, fFloat ZValue = 1.0f);
	fResult Clear(const fcyColor& BackBufferColor = 0, fFloat ZValue = 1.0f, fuInt StencilValue = 0);
	fResult ClearColor(const fcyColor& BackBufferColor = 0);
	fResult ClearZBuffer(fFloat Value=1.0f);
	fResult ClearStencilBuffer(fuInt StencilValue=0);
	f2dTexture2D* GetRenderTarget();
	fResult SetRenderTarget(f2dTexture2D* pTex);
	f2dDepthStencilSurface* GetDepthStencilSurface();
	fResult SetDepthStencilSurface(f2dDepthStencilSurface* pSurface);
	fcyRect GetScissorRect();
	fResult SetScissorRect(const fcyRect& pRect);
	fcyRect GetViewport();
	fResult SetViewport(fcyRect vp);
	fBool IsZBufferEnabled();
	fResult SetZBufferEnable(fBool v);

	fResult UpdateScreenToWindow(fcyColor KeyColor, fByte Alpha);
	fResult SaveScreen(f2dStream* pStream) { return FCYERR_NOTSUPPORT; }
	fResult SaveTexture(f2dStream* pStream, f2dTexture2D* pTex) { return FCYERR_NOTSUPPORT; }
	fResult SaveScreen(fcStrW path);
	fResult SaveTexture(fcStrW path, f2dTexture2D* pTex);
	
	//纹理采样设置
	fResult SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor);
	fResult SetTextureFilter(F2DTEXFILTERTYPE filter);
public:
	f2dRenderDeviceImpl(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel);
	~f2dRenderDeviceImpl();
};
