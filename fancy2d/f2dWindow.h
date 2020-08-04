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
/// @brief fancy2d渲染窗口IME信息
////////////////////////////////////////////////////////////////////////////////
enum F2DIMEINFO
{
	F2DIMEINFO_NULL,
	F2DIMEINFO_CANDIDATECOUNT,  ///< @brief 候选词个数
	F2DIMEINFO_CANDIDATEINDEX,  ///< @brief 当前选中的候选词索引
	F2DIMEINFO_PAGESIZE,        ///< @brief 一页的候选词个数
	F2DIMEINFO_PAGESTART        ///< @brief 当前页面开始的候选词索引
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2d候选词列表
////////////////////////////////////////////////////////////////////////////////
struct f2dIMECandidateList
{
	virtual fuInt GetCount()=0;     ///< @brief 返回候选词数量
	virtual fuInt GetCurIndex()=0;  ///< @brief 返回当前候选词索引
	virtual fuInt GetPageSize()=0;  ///< @brief 返回单个页面候选词数量
	virtual fuInt GetPageStart()=0; ///< @brief 返回当前页面首个候选词的索引
	
	/// @brief 查询候选词
	virtual fcStrW GetCandidateStr(fuInt Index)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染窗口事件监听器
/// @note  通过覆写这个类来实现消息回调
////////////////////////////////////////////////////////////////////////////////
struct f2dWindowEventListener
{
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

	/// @brief 输入法开始拼写
	virtual void OnIMEStartComposition() {}

	/// @brief 输入法结束拼写
	virtual void OnIMEEndComposition() {}

	/// @brief     输入法正在拼写
	/// @param[in] String   拼写串的文本指针，亦可由窗口接口获得
	/// @param[in] CharCode 输入的字符
	virtual void OnIMEComposition(fcStrW String, fCharW CharCode) {}

	/// @brief     输入法激活
	/// @param[in] Desc 输入法描述，亦可由窗口接口获得
	virtual void OnIMEActivated(fcStrW Desc) {}

	/// @brief 输入法关闭
	virtual void OnIMEClosed() {}

	/// @brief     输入法候选词改变
	/// @param[in] pList 候选词列表，亦可由窗口接口获得
	virtual void OnIMEChangeCandidate(f2dIMECandidateList* pList) {}

	/// @brief 输入法候选词窗口打开
	/// @param[in] pList 候选词列表，亦可由窗口接口获得
	virtual void OnIMEOpenCandidate(f2dIMECandidateList* pList) {}

	/// @brief 输入法候选词窗口打开
	/// @param[in] pList 候选词列表，亦可由窗口接口获得
	virtual void OnIMECloseCandidate(f2dIMECandidateList* pList) {}

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
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染窗口
////////////////////////////////////////////////////////////////////////////////
struct f2dWindow
{
	/// @brief 返回监听器
	virtual f2dWindowEventListener* GetListener()=0;

	/// @brief 设置监听器
	virtual fResult SetListener(f2dWindowEventListener* pListener)=0;

	/// @brief   返回Windows窗口句柄
	/// @note    需要强转到HWND
	/// @warning 如无特殊情况应当避免使用本方法
	virtual fInt GetHandle()=0;

	/// @brief 返回边框类型
	virtual F2DWINBORDERTYPE GetBorderType()=0;

	/// @brief 设置边框类型
	virtual fResult SetBorderType(F2DWINBORDERTYPE Type)=0;

	/// @brief 返回窗口标题
	virtual fcStrW GetCaption()=0;

	/// @brief 设置窗口标题
	/// @note  不超过1024个字符
	virtual fResult SetCaption(fcStrW Caption)=0;

	/// @brief 返回是否可见
	virtual fBool GetVisiable()=0;

	/// @brief 设置是否可见
	virtual fResult SetVisiable(fBool Visiable)=0;

	/// @brief 返回窗口位置（含边框）
	virtual fcyRect GetRect()=0;

	/// @brief 设置窗口位置（含边框）
	virtual fResult SetRect(const fcyRect& Range)=0;

	/// @brief 返回窗口客户区位置
	virtual fcyRect GetClientRect()=0;

	/// @brief 设置窗口客户区位置
	virtual fResult SetClientRect(const fcyRect& Range)=0;

	/// @brief 移动到屏幕中心
	virtual void MoveToCenter()=0;

	/// @brief 是否置顶
	virtual fBool IsTopMost()=0;

	/// @brief 设置置顶
	virtual fResult SetTopMost(fBool TopMost)=0;
	
	/// @brief     隐藏鼠标
	/// @param[in] bHide 是否隐藏鼠标
	virtual void HideMouse(fBool bHide)=0;

	/// @brief 隐藏IME
	virtual fBool IsHideIME()=0;
	
	/// @brief 设置是否隐藏IME
	virtual void SetHideIME(fBool bHide)=0;

	/// @brief 返回IME描述
	virtual fcStrW GetIMEDesc()=0;

	/// @brief 返回IME信息
	virtual fuInt GetIMEInfo(F2DIMEINFO InfoType)=0;

	/// @brief 返回当前的拼写串
	virtual fcStrW GetIMECompString()=0;

	/// @brief 返回候选词个数
	virtual fuInt GetIMECandidateCount()=0;

	/// @brief 返回候选词
	virtual fcStrW GetIMECandidate(fuInt Index)=0;
};

/// @}
