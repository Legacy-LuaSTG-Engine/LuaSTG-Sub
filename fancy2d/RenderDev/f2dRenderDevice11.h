////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDeviceImpl.h
/// @brief fancy2D渲染设备接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dEngine.h"
#include "Engine/f2dWindowImpl.h"
#include "Common/f2dStandardCommon.hpp"
#include "Common/f2dWindowsCommon.h"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染设备实现
////////////////////////////////////////////////////////////////////////////////
class f2dRenderDevice11 :
	public fcyRefObjImpl<f2dRenderDevice>,
	public f2dWindowEventListener
{
private:
	struct EventListenerNode
	{
		int uuid;
		int priority;
		f2dRenderDeviceEventListener* listener;
		
		bool operator<(const EventListenerNode& rhs) const
		{
			return priority == rhs.priority ? uuid < rhs.uuid : priority < rhs.priority;
		}
	};
	struct DXGISwapchainResizeData
	{
		UINT BufferCount = 0;
		DXGI_FORMAT Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		UINT Flags = 0;
		BOOL AllowTearing = FALSE;
	};
private:
	f2dEngineImpl* m_pEngine = nullptr;

	int _iEventListenerUUID = 0;
	std::set<EventListenerNode> _setEventListeners;
	
	bool m_bDevLost = false; // 设备丢失标志
	f2dGraphics* m_pCurGraphics = nullptr; // 当前的绘图对象，只记录指针
	
	DWORD m_CreateThreadID = 0;
	HWND m_hWnd = NULL;
	std::string m_DevName;
	
	HWND win32_window = NULL;

	fuInt swapchain_width = 0;
	fuInt swapchain_height = 0;
	fBool swapchain_windowed = true;
	fBool swapchain_vsync = false;
	fBool swapchain_flip = false;
	fBool swapchain_want_exit_fullscreen = false;
	fBool swapchain_want_enter_fullscreen = false;
	fBool swapchain_want_resize = false;
	DXGISwapchainResizeData swapchain_resize_data;
	std::vector<f2dDisplayMode> display_modes;

	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;

	Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_devctx;
	D3D_FEATURE_LEVEL d3d11_level = D3D_FEATURE_LEVEL_10_0;
	
	Microsoft::WRL::ComPtr<IDXGISwapChain> dxgi_swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11_rendertarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> d3d11_depthstencil;

	fcyRefPointer<f2dTexture2D> m_RenderTarget;
	fcyRefPointer<f2dDepthStencilSurface> m_DepthStencil;

	fBool d3d11_support_bgra = false;
	fBool dxgi_support_tearing = false;
private:
	int sendDevLostMsg();             // 发送设备丢失事件, 返回对象数目
	int sendDevResetMsg();            // 发送设备重置事件
	int dispatchRenderSizeDependentResourcesCreate();
	int dispatchRenderSizeDependentResourcesDestroy();
	bool selectAdapter();
	bool checkFeatureSupported();
	void beforeDestroyDevice();
	void destroySwapchain();
	bool createSwapchain(f2dDisplayMode* mode);
	void destroyRenderAttachments();
	bool createRenderAttachments();
	void setupRenderAttachments();
public: // 内部函数
	f2dEngineImpl* GetEngine() { return m_pEngine; } // 返回引擎对象
	fResult SyncDevice();                           // 协作测试，完成设备丢失处理
	fResult Present();                              // 呈现
	f2dGraphics* QueryCurGraphics();
	fResult SubmitCurGraphics(f2dGraphics* pGraph, bool bDirty);

	// 废弃方法集合
	fResult SubmitWorldMat(const fcyMatrix4& Mat);
	fResult SubmitLookatMat(const fcyMatrix4& Mat);
	fResult SubmitProjMat(const fcyMatrix4& Mat);
	fResult SubmitBlendState(const f2dBlendState& State);
	fResult SubmitVD(IDirect3DVertexDeclaration9* pVD);
	fResult SubmitTextureBlendOP_Color(D3DTEXTUREOP ColorOP);
public:
	// 用于全屏

	void OnLostFocus();
	void OnGetFocus();
	void OnSize(fuInt ClientWidth, fuInt ClientHeight);
public:
	// 接口实现

	void* GetHandle();
	fcStr GetDeviceName();
	
	fResult AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority = 0);
	fResult RemoveListener(f2dRenderDeviceEventListener* Listener);

	fuInt GetSupportedDisplayModeCount(fBool refresh);
	f2dDisplayMode GetSupportedDisplayMode(fuInt Index);
	fResult SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel);
	fResult SetDisplayMode(f2dDisplayMode mode, fBool VSync);
	fuInt GetBufferWidth();
	fuInt GetBufferHeight();
	fBool IsWindowed();
	
	fResult CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut);
	fResult CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize , f2dTexture2D** pOut);
	fResult CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut);
	fResult CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut);
	
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

	fResult SaveScreen(f2dStream* pStream);
	fResult SaveTexture(f2dStream* pStream, f2dTexture2D* pTex);
	fResult SaveScreen(fcStrW path);
	fResult SaveTexture(fcStrW path, f2dTexture2D* pTex);
	
	// 废弃方法集合

	F2DAALEVEL GetAALevel();
	fBool CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed);
	fResult CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut);
	fResult CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut);
	fResult CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut);
	fBool IsZBufferEnabled();
	fResult SetZBufferEnable(fBool v);
	fResult UpdateScreenToWindow(fcyColor KeyColor, fByte Alpha);
	fResult SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor);
	fResult SetTextureFilter(F2DTEXFILTERTYPE filter);
	fResult SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel);
	fResult SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRateA, fuInt RefreshRateB, fBool Windowed, fBool VSync, fBool FlipModel);
public:
	f2dRenderDevice11(f2dEngineImpl* pEngine, fuInt BackBufferWidth, fuInt BackBufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel);
	~f2dRenderDevice11();
};
