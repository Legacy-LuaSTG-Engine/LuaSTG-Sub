////////////////////////////////////////////////////////////////////////////////
/// @file  f2dEngineImpl.h
/// @brief fancy2D引擎接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <fcyRefObj.h>
#include "f2dEngine.h"
#include "Engine/f2dWindowImpl.h"
#include "Sound/f2dSoundSysImpl.h"
#include "Renderer/f2dRendererImpl.h"
#include "Core/ApplicationModel_Win32.hpp"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D引擎接口实现
////////////////////////////////////////////////////////////////////////////////
class f2dEngineImpl :
	public fcyRefObjImpl<f2dEngine>
{
private:
	LuaSTG::Core::ScopeObject<LuaSTG::Core::ApplicationModel_Win32> m_appmodel;

	// 组件
	f2dWindowImpl*   m_pWindow;
	f2dSoundSysImpl* m_pSoundSys;
	f2dRendererImpl* m_pRenderer;

public: // 接口实现
	fResult InitWindow(const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border);
	fResult InitSoundSys();
	fResult InitRenderer(f2dEngineRenderWindowParam* RenderWindowParam);

	f2dWindow* GetMainWindow() { return m_pWindow;   }
	f2dSoundSys* GetSoundSys() { return m_pSoundSys; }
	f2dRenderer* GetRenderer() { return m_pRenderer; }

	void* GetAppModel() { return *m_appmodel; }
	LuaSTG::Core::ApplicationModel_Win32* GGetAppModel() { return *m_appmodel; }
public:
	f2dEngineImpl(f2dEngineRenderWindowParam* RenderWindowParam);
	~f2dEngineImpl();
};
