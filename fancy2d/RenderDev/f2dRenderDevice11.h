////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDeviceImpl.h
/// @brief fancy2D渲染设备接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dEngine.h"
#include "Core/Graphics/Device_D3D11.hpp"
#include "Core/Graphics/SwapChain_D3D11.hpp"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染设备实现
////////////////////////////////////////////////////////////////////////////////
class f2dRenderDevice11
	: public fcyRefObjImpl<f2dRenderDevice>
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
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::Device_D3D11> m_pGraphicsDevice;
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::SwapChain_D3D11> m_pSwapChain;

	int _iEventListenerUUID = 0;
	std::set<EventListenerNode> _setEventListeners;
	
	f2dGraphics* m_pCurGraphics = nullptr; // 当前的绘图对象，只记录指针
	
public:
	// 事件监听

	void onDeviceCreate();
	void onDeviceDestroy();

	void onSwapChainCreate();
	void onSwapChainDestroy();
public:
	// 接口实现

	void* GetHandle();
	LuaSTG::Core::Graphics::Device_D3D11* GetDevice() { return *m_pGraphicsDevice; }
	LuaSTG::Core::Graphics::SwapChain_D3D11* GetSwapChain() { return *m_pSwapChain; }
	fuInt GetSupportedDeviceCount();
	fcStr GetSupportedDeviceName(fuInt Index);

	fResult AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority = 0);
	fResult RemoveListener(f2dRenderDeviceEventListener* Listener);

	fResult SetDisplayMode(fuInt Width, fuInt Height, fBool VSync, fBool FlipModel);
	fResult SetDisplayMode(f2dDisplayMode mode, fBool VSync);
	fuInt GetBufferWidth();
	fuInt GetBufferHeight();

	fResult SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, fBool FlipModel, F2DAALEVEL AALevel);
	fResult SetDisplayMode(fuInt Width, fuInt Height, fuInt RefreshRateA, fuInt RefreshRateB, fBool Windowed, fBool VSync, fBool FlipModel);

	fResult CreateTextureFromFile(fcStr path, fBool HasMipmap, f2dTexture2D** pOut);
	fResult CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut);
	
public:
	f2dRenderDevice11(f2dEngineImpl* pEngine, f2dEngineRenderWindowParam* RenderWindowParam);
	~f2dRenderDevice11();
};
