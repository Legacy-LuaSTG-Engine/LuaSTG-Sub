////////////////////////////////////////////////////////////////////////////////
/// @file  f2dWindowImpl.h
/// @brief fancy2D渲染窗口接口实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO\fcyStream.h"
#include "f2dWindow.h"
#include "Core/Graphics/Window_Win32.hpp"

class f2dEngineImpl;

////////////////////////////////////////////////////////////////////////////////
/// @brief 渲染窗口
////////////////////////////////////////////////////////////////////////////////
class f2dWindowImpl
	: public fcyRefObjImpl<f2dWindow>
	, public LuaSTG::Core::Graphics::IWindowEventListener
{
private:
	// 监听器
	LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::Window_Win32> m_window;
	std::vector<fHandle> m_fProc;
	void onNativeWindowMessage(void* w, uint32_t m, uintptr_t a, intptr_t b) { HandleNativeMessageCallback((HWND)w, m, a, b); }
public:
	// 接口实现

	void AddNativeMessageCallback(fHandle pWndProc);
	void RemoveNativeMessageCallback(fHandle pWndProc);
	bool HandleNativeMessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	fHandle GetHandle();
	
	fResult SetBorderType(F2DWINBORDERTYPE Type);
	fResult SetCaption(fcStr Caption);
	fResult SetVisiable(fBool Visiable);
	fResult SetClientRect(const fcyRect& Range);
	fResult SetTopMost(fBool TopMost);
	void HideMouse(fBool bShow);
	
	void SetIMEEnable(bool enable);
	
	float GetDPIScaling();

	void MoveToCenter();
	void EnterFullScreen();
	fcyVec2 GetMonitorSize();

	fuInt GetMonitorCount();
	fcyRect GetMonitorRect(fuInt index);
	void MoveToMonitorCenter(fuInt index);
	void EnterMonitorFullScreen(fuInt index);
	
	void SetCustomMoveSizeEnable(fBool v);
	void SetCustomMinimizeButtonRect(fcyRect v);
	void SetCustomCloseButtonRect(fcyRect v);
	void SetCustomMoveButtonRect(fcyRect v);
public:
	f2dWindowImpl(f2dEngineImpl* pEngine, const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border, bool DisableIME = true);
	~f2dWindowImpl();
};
