////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDeviceImpl.h
/// @brief fancy2D渲染设备接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dEngine.h"
#include "Engine/f2dWindowImpl.h"
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/SwapChain.hpp"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染设备实现
////////////////////////////////////////////////////////////////////////////////
class f2dRenderDevice11
	: public fcyRefObjImpl<f2dRenderDevice>
	, public f2dWindowEventListener
	, public LuaSTG::Core::Graphics::IDeviceEventListener
	, public LuaSTG::Core::Graphics::ISwapChainEventListener
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
private:
	f2dEngineImpl* m_pEngine = nullptr;
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IDevice> m_pGraphicsDevice;
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ISwapChain> m_pSwapChain;

	int _iEventListenerUUID = 0;
	std::set<EventListenerNode> _setEventListeners;
	
	f2dGraphics* m_pCurGraphics = nullptr; // 当前的绘图对象，只记录指针
	
	HWND win32_window = NULL;

	fBool swapchain_want_exit_fullscreen = false;
	fBool swapchain_want_enter_fullscreen = false;
	fBool swapchain_want_resize = false;

	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgi_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter;

	Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_devctx;
	
	fcyRefPointer<f2dTexture2D> m_RenderTarget;
	fcyRefPointer<f2dDepthStencilSurface> m_DepthStencil;

private:
	int sendDevLostMsg();             // 发送设备丢失事件, 返回对象数目
	int sendDevResetMsg();            // 发送设备重置事件
	int dispatchRenderSizeDependentResourcesCreate();
	int dispatchRenderSizeDependentResourcesDestroy();
public: // 内部函数
	f2dEngineImpl* GetEngine() { return m_pEngine; } // 返回引擎对象
	fResult WaitDevice();
	fResult SyncDevice();                           // 协作测试，完成设备丢失处理
	fResult Present();                              // 呈现
	f2dGraphics* QueryCurGraphics();
	fResult SubmitCurGraphics(f2dGraphics* pGraph, bool bDirty);
public:
	// 事件监听

	void onDeviceCreate();
	void onDeviceDestroy();

	void onSwapChainCreate();
	void onSwapChainDestroy();

	void OnLostFocus();
	void OnGetFocus();
	void OnSize(fuInt ClientWidth, fuInt ClientHeight);
public:
	// 接口实现

	void* GetHandle();
	fcStr GetDeviceName();
	fuInt GetSupportedDeviceCount();
	fcStr GetSupportedDeviceName(fuInt Index);
	f2dAdapterMemoryUsageStatistics GetAdapterMemoryUsageStatistics();
	
	fResult AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority = 0);
	fResult RemoveListener(f2dRenderDeviceEventListener* Listener);

	fuInt GetSupportedDisplayModeCount(fBool refresh);
	f2dDisplayMode GetSupportedDisplayMode(fuInt Index);
	fResult SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel);
	fResult SetDisplayMode(f2dDisplayMode mode, fBool VSync);
	fuInt GetBufferWidth();
	fuInt GetBufferHeight();
	fBool IsWindowed();
	
	fResult SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel);
	fResult SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRateA, fuInt RefreshRateB, fBool Windowed, fBool VSync, fBool FlipModel);

	fResult CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut);
	fResult CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize , f2dTexture2D** pOut);
	fResult CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut);
	
	f2dTexture2D* GetRenderTarget();
	f2dDepthStencilSurface* GetDepthStencilSurface();
	fResult SetRenderTargetAndDepthStencilSurface(f2dTexture2D* pTex, f2dDepthStencilSurface* pSurface);

	fResult SaveScreen(f2dStream* pStream);
	fResult SaveTexture(f2dStream* pStream, f2dTexture2D* pTex);
	fResult SaveScreen(fcStrW path);
	fResult SaveTexture(fcStrW path, f2dTexture2D* pTex);
	
	// 废弃方法集合
	fResult CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut);
	fResult CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut);
	fResult CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut);
	fResult CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut);
public:
	f2dRenderDevice11(f2dEngineImpl* pEngine, f2dEngineRenderWindowParam* RenderWindowParam);
	~f2dRenderDevice11();
};
