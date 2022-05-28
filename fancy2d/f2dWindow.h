////////////////////////////////////////////////////////////////////////////////
/// @file  f2dWindow.h
/// @brief fancy2D渲染窗口接口定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

/// @addtogroup f2d渲染窗口
/// @brief fancy2d引擎渲染窗口
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2d渲染窗口边框类型
////////////////////////////////////////////////////////////////////////////////
enum F2DWINBORDERTYPE
{
	F2DWINBORDERTYPE_NONE,
	F2DWINBORDERTYPE_FIXED,
	F2DWINBORDERTYPE_SIZEABLE
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染窗口事件监听器
/// @note  通过覆写这个类来实现消息回调
////////////////////////////////////////////////////////////////////////////////
struct f2dWindowEventListener
{
#pragma warning(disable:4100)

	/// @brief 窗口关闭事件
	virtual void OnClose() {}

	/// @brief 窗口重绘事件
	virtual void OnPaint() {}

	/// @brief     窗口大小改变事件
	/// @param[in] ClientWidth  新的客户区宽度
	/// @param[in] ClientHeight 新的客户区高度
	virtual void OnSize(fuInt ClientWidth, fuInt ClientHeight) {}

	/// @brief     键被按下
	/// @param[in] KeyCode 按键代码，请查阅MSDN
	/// @param[in] Flag    附带属性，请查阅MSDN
	virtual void OnKeyDown(fuInt KeyCode, fuInt Flag) {}

	/// @brief     键被放开
	/// @param[in] KeyCode 按键代码，请查阅MSDN
	/// @param[in] Flag    附带属性，请查阅MSDN
	virtual void OnKeyUp(fuInt KeyCode, fuInt Flag) {}

	/// @brief     字符输入
	/// @param[in] CharCode 字符
	/// @param[in] Flag     附带属性，请查阅MSDN
	virtual void OnCharInput(fCharW CharCode, fuInt Flag) {}

	/// @brief     鼠标移动
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseMove(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     滚轮滚动
	/// @param[in] X     鼠标横向位置
	/// @param[in] Y     鼠标纵向位置
	/// @param[in] Wheel 滚轮滚动数量
	/// @param[in] Flag  附带属性，请查阅MSDN
	virtual void OnMouseWheel(fShort X, fShort Y, fFloat Wheel, fuInt Flag) {}

	/// @brief     鼠标左键按下
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseLBDown(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标左键放开
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseLBUp(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标左键双击
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseLBDouble(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标中键按下
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseMBDown(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标中键放开
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseMBUp(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标中键双击
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseMBDouble(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标右键按下
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseRBDown(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标右键放开
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseRBUp(fShort X, fShort Y, fuInt Flag) {}

	/// @brief     鼠标右键双击
	/// @param[in] X    鼠标横向位置
	/// @param[in] Y    鼠标纵向位置
	/// @param[in] Flag 附带属性，请查阅MSDN
	virtual void OnMouseRBDouble(fShort X, fShort Y, fuInt Flag) {}

	/// @brief 获得焦点
	virtual void OnGetFocus() {}

	/// @brief 丢失焦点
	virtual void OnLostFocus() {}
	
	// 系统设备变化
	virtual void OnDeviceChange() {}

#pragma warning(default:4100)
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染窗口
////////////////////////////////////////////////////////////////////////////////
struct f2dWindow
{
	virtual void AddNativeMessageCallback(fHandle pWndProc)=0;
	virtual void RemoveNativeMessageCallback(fHandle pWndProc)=0;
	
	virtual fHandle GetIWindow() = 0;
	virtual fHandle GetHandle()=0;
	virtual fResult SetBorderType(F2DWINBORDERTYPE Type)=0;
	virtual fResult SetCaption(fcStr Caption)=0;
	virtual fResult SetVisiable(fBool Visiable)=0;
	virtual fResult SetClientRect(const fcyRect& Range)=0;
	virtual fResult SetTopMost(fBool TopMost)=0;
	virtual void HideMouse(fBool bHide)=0;
	virtual void SetIMEEnable(bool enable)=0;
	virtual float GetDPIScaling()=0;
	
	virtual fcyVec2 GetMonitorSize()=0;
	virtual void MoveToCenter()=0;
	virtual void EnterFullScreen()=0;

	virtual fuInt GetMonitorCount() = 0;
	virtual fcyRect GetMonitorRect(fuInt index) = 0;
	virtual void MoveToMonitorCenter(fuInt index) = 0;
	virtual void EnterMonitorFullScreen(fuInt index) = 0;
	
	virtual void SetCustomMoveSizeEnable(fBool v) = 0;
	virtual void SetCustomMinimizeButtonRect(fcyRect v) = 0;
	virtual void SetCustomCloseButtonRect(fcyRect v) = 0;
	virtual void SetCustomMoveButtonRect(fcyRect v) = 0;
};

/// @}
