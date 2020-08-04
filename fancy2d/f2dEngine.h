////////////////////////////////////////////////////////////////////////////////
/// @file  f2dEngine.h
/// @brief fancy2D引擎接口定义
/// @note  定义了f2D的引擎接口
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

// 所有组件
#include "f2dFileSys.h"
#include "f2dWindow.h"
#include "f2dInputSys.h"
#include "f2dSoundSys.h"
#include "f2dRenderer.h"
#include "f2dVideoSys.h"

/// @addtogroup f2d引擎
/// @brief fancy2d引擎接口
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D CPU信息
////////////////////////////////////////////////////////////////////////////////
struct f2dCPUInfo
{
	fcStr CPUString;      ///< @brief 标识
	fcStr CPUBrandString; ///< @brief 商品名
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D FPS控制器
/// @note  非线程安全
////////////////////////////////////////////////////////////////////////////////
struct f2dFPSController
{
	/// @brief 返回最大FPS
	/// @note  0表示不限制
	virtual fuInt GetLimitedFPS()=0;
	
	/// @brief 设置最大FPS
	/// @param iMaxFPS 最大FPS
	virtual void SetLimitedFPS(fuInt iMaxFPS)=0;
	
	/// @brief 返回FPS
	virtual fDouble GetFPS()=0;

	/// @brief 返回总帧数
	virtual fuInt GetTotalFrame()=0;
	
	/// @brief 返回运行总时间
	/// @note  以秒为单位
	virtual fDouble GetTotalTime()=0;
	
	/// @brief 返回平均FPS
	virtual fDouble GetAvgFPS()=0;
	
	/// @brief 返回最大FPS
	virtual fDouble GetMaxFPS()=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 消息类型
////////////////////////////////////////////////////////////////////////////////
enum F2DMSGTYPE
{
	F2DMSG_USER = 0,                      ///< @brief 用户消息
	
	F2DMSG_APP_ONEXIT      = 1,           ///< @brief 程序退出消息
	F2DMSG_APP_ONEXCEPTION = 2,           ///< @brief 程序异常消息 (fuInt Time,fcStr Src,fcStr Desc)

	F2DMSG_WINDOW_ONCLOSE        = 10,    ///< @brief 窗口关闭消息
	F2DMSG_WINDOW_ONPAINT        = 11,    ///< @brief 窗口重绘消息
	F2DMSG_WINDOW_ONGETFOCUS     = 12,    ///< @brief 获得焦点消息
	F2DMSG_WINDOW_ONLOSTFOCUS    = 13,    ///< @brief 丢失焦点消息
	F2DMSG_WINDOW_ONRESIZE       = 14,    ///< @brief 窗口缩放消息 (fuInt Width, fuInt Height)
	F2DMSG_WINDOW_ONCHARINPUT    = 15,    ///< @brief 字符输入消息 (fCharW Char)
	F2DMSG_WINDOW_ONMOUSEMOVE    = 16,    ///< @brief 鼠标移动     (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSELUP     = 17,    ///< @brief 鼠标左键放开 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSELDOWN   = 18,    ///< @brief 鼠标左键按下 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSELDOUBLE = 19,    ///< @brief 鼠标左键双击 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSERUP     = 20,    ///< @brief 鼠标右键放开 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSERDOWN   = 21,    ///< @brief 鼠标右键按下 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSERDOUBLE = 22,    ///< @brief 鼠标右键双击 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSEMUP     = 23,    ///< @brief 鼠标中键放开 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSEMDOWN   = 24,    ///< @brief 鼠标中键按下 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSEMDOUBLE = 25,    ///< @brief 鼠标中键双击 (fInt X, fInt Y)
	F2DMSG_WINDOW_ONMOUSEWHEEL   = 26,    ///< @brief 鼠标滚动     (fInt X, fInt Y, fDouble Wheel)
	F2DMSG_WINDOW_ONKEYUP        = 27,    ///< @brief 按键放开     (fuInt KeyCode)
	F2DMSG_WINDOW_ONKEYDOWN      = 28,    ///< @brief 按键按下     (fuInt KeyCode)

	F2DMSG_IME_ONCLOSE            = 30,   ///< @brief IME关闭
	F2DMSG_IME_ONACTIVATE         = 31,   ///< @brief IME激活       (fcStrW Desc)
	F2DMSG_IME_ONSTARTCOMPOSITION = 32,   ///< @brief IME开始拼写
	F2DMSG_IME_ONENDCOMPOSITION   = 33,   ///< @brief IME结束拼写
	F2DMSG_IME_ONCOMPOSITION      = 34,   ///< @brief IME正在拼写   (fcStrW CompStr, fCharW Char)
	F2DMSG_IME_ONOPENCANDIDATE    = 35,   ///< @brief IME打开候选词 (f2dIMECandidateList* Ptr)
	F2DMSG_IME_ONCLOSECANDIDATE   = 36,   ///< @brief IME关闭候选词 (f2dIMECandidateList* Ptr)
	F2DMSG_IME_ONCHANGECANDIDATE  = 37,   ///< @brief IME改变候选词 (f2dIMECandidateList* Ptr)

	F2DMSG_RENDER_ONDEVLOST   = 50,       ///< @brief 设备丢失消息
	F2DMSG_RENDER_ONDEVRESET  = 51,       ///< @brief 设备重置消息 
	
	F2DMSG_MOUSE_ONOFFSET     = 60,       ///< @brief 鼠标位移消息 (fInt X, fInt Y, fInt Z)
	F2DMSG_MOUSE_ONLBTNUP     = 61,       ///< @brief 鼠标左键放开
	F2DMSG_MOUSE_ONLBTNDOWN   = 62,       ///< @brief 鼠标左键按下
	F2DMSG_MOUSE_ONRBTNUP     = 63,       ///< @brief 鼠标右键放开
	F2DMSG_MOUSE_ONRBTNDOWN   = 64,       ///< @brief 鼠标右键按下
	F2DMSG_MOUSE_ONMBTNUP     = 65,       ///< @brief 鼠标中键放开
	F2DMSG_MOUSE_ONMBTNDOWN   = 66,       ///< @brief 鼠标中键按下
	F2DMSG_MOUSE_ONADDBTNUP   = 67,       ///< @brief 鼠标额外按键放开 (fuInt Index)
	F2DMSG_MOUSE_ONADDBTNDOWN = 68,       ///< @brief 鼠标额外按键按下 (fuInt Index)
	
	F2DMSG_KEYBOARD_ONKEYUP   = 70,       ///< @brief 按键放开 (F2DINPUTKEYCODE Key)
	F2DMSG_KEYBOARD_ONKEYDOWN = 71,       ///< @brief 按键按下 (F2DINPUTKEYCODE Key)

	F2DMSG_JOYSTICK_ONBUTTONUP   = 80,    ///< @brief 手柄按键放开 (fuInt Index, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONBUTTONDOWN = 81,    ///< @brief 手柄按键按下 (fuInt Index, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONXPOSCHANGE = 82,    ///< @brief X轴位置改变  (fDouble Value, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONYPOSCHANGE = 83,    ///< @brief Y轴位置改变  (fDouble Value, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONZPOSCHANGE = 84,    ///< @brief Z轴位置改变  (fDouble Value, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONXROTCHANGE = 85,    ///< @brief X轴旋转改变  (fDouble Value, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONYROTCHANGE = 86,    ///< @brief Y轴旋转改变  (fDouble Value, f2dInputJoystick*)
	F2DMSG_JOYSTICK_ONZROTCHANGE = 87     ///< @brief Z轴旋转改变  (fDouble Value, f2dInputJoystick*)
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 消息
////////////////////////////////////////////////////////////////////////////////
struct f2dMsg
{
	F2DMSGTYPE Type;  ///< @brief 消息类型
	fuLong Param1;    ///< @brief 参数1
	fuLong Param2;    ///< @brief 参数2
	fuLong Param3;    ///< @brief 参数3
	fuLong Param4;    ///< @brief 参数4
};

////////////////////////////////////////////////////////////////////////////////
/// @brief   fancy2D 消息泵
/// @note    消息泵会在Update结束后清空所有消息并回收额外内存
/// @warning 非线程安全
////////////////////////////////////////////////////////////////////////////////
struct f2dMsgPump
{
	/// @brief 消息泵是否空
	virtual fBool IsEmpty()=0;
	/// @brief 弹出一个消息
	virtual fResult GetMsg(f2dMsg* MsgOut)=0;
	/// @brief 查询一个消息
	virtual fResult PeekMsg(f2dMsg* MsgOut)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 主线程委托
/// @note  允许将方法委托给主线程运行
////////////////////////////////////////////////////////////////////////////////
struct f2dMainThreadDelegate : public f2dInterface
{
	/// @brief 执行函数
	virtual void Excute()=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D引擎事件监听器
/// @note  通过覆写这个类来实现消息回调
////////////////////////////////////////////////////////////////////////////////
struct f2dEngineEventListener
{
	/// @brief     更新一帧
	/// @note      应当在这里完成更新操作
	/// @warning   该方法可能跨线程
	/// @param[in] ElapsedTime 距离上一帧的流逝时间
	/// @return    返回false退出循环
	virtual fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump) { return true; }
	
	/// @brief     渲染一帧
	/// @note      仅能在这里完成渲染操作
	/// @warning   该方法可能跨线程
	/// @param[in] ElapsedTime 距离上一帧的流逝时间
	/// @return    返回true更新画面
	virtual fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController) { return true; }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D引擎多线程模式
////////////////////////////////////////////////////////////////////////////////
enum F2DENGTHREADMODE
{
	F2DENGTHREADMODE_SINGLETHREAD,    ///< @brief 单线程模式
	F2DENGTHREADMODE_MULTITHREAD,     ///< @brief OnUpdate和OnRender在同一线程，共用一个FPS控制器
	F2DENGTHREADMODE_FULLMULTITHREAD  ///< @brief OnUpdate和OnRender分跨两个线程，不共用FPS控制器
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D引擎
////////////////////////////////////////////////////////////////////////////////
struct f2dEngine : f2dInterface
{
	// --- 监听器 ---
	/// @brief 返回监听器
	virtual f2dEngineEventListener* GetListener()=0;

	/// @brief 设置监听器
	virtual fResult SetListener(f2dEngineEventListener* pListener)=0;

	// --- 组件初始化 ---
	/// @brief     初始化渲染窗口
	/// @warning   禁止跨线程初始化
	/// @note      渲染窗口必须在所有组件之前初始化
	/// @param[in] Pos      客户区位置
	/// @param[in] Title    窗口标题
	/// @param[in] Visiable 可见
	/// @param[in] Border   边框
	virtual fResult InitWindow(const fcyRect& Pos, fcStrW Title, fBool Visiable, F2DWINBORDERTYPE Border)=0;
	
	/// @brief 初始化声音系统
	/// @note  需要预先初始化渲染窗口
	virtual fResult InitSoundSys()=0;

	/// @brief 初始化输入系统
	/// @note  需要预先初始化渲染窗口
	virtual fResult InitInputSys()=0;

	/// @brief     初始化渲染系统
	/// @warning   禁止跨线程初始化
	/// @note      需要预先初始化渲染窗口
	/// @param[in] BufferWidth  缓冲区宽度
	/// @param[in] BufferHeight 缓冲区高度
	/// @param[in] Windowed     窗口化
	/// @param[in] VSync        垂直同步
	/// @param[in] AALevel      抗锯齿等级
	virtual fResult InitRenderer(fuInt BufferWidth, fuInt BufferHeight, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)=0;

	/// @brief 初始化视频系统
	/// @note  需要渲染系统支持
	virtual fResult InitVideoSys()=0;

	// --- 获得组件 ---
	virtual fcyCriticalSection& GetCriticalSection() = 0;
	virtual f2dWindow* GetMainWindow()=0;  ///< @brief 获得主窗口
	virtual f2dFileSys* GetFileSys()=0;    ///< @brief 获得文件系统
	virtual f2dSoundSys* GetSoundSys()=0;  ///< @brief 获得声音系统
	virtual f2dInputSys* GetInputSys()=0;  ///< @brief 获得输入系统
	virtual f2dRenderer* GetRenderer()=0;  ///< @brief 获得渲染系统
	virtual f2dVideoSys* GetVideoSys()=0;  ///< @brief 获得视频系统

	// --- 错误 ---
	virtual void ClearLstErr()=0;      ///< @brief 清空上一次错误
	virtual fuInt GetLastErrTime()=0;  ///< @brief 返回上一次错误的时间
	virtual fcStr GetLastErrSrc()=0;   ///< @brief 返回上一次错误的来源
	virtual fcStr GetLastErrDesc()=0;  ///< @brief 返回上一次错误的描述

	// --- 引擎控制 ---
	/// @brief 终止程序循环
	/// @note  线程安全
	/// @note  无论Update返回结果如何，在一帧结束后终止循环
	virtual void Abort()=0;

	/// @brief 执行程序循环
	/// @param[in] ThreadMode   线程模式
	/// @param[in] UpdateMaxFPS 更新线程最大FPS
	/// @param[in] RenderMaxFPS 渲染线程最大FPS（仅完全多线程模式有效）
	virtual void Run(F2DENGTHREADMODE ThreadMode, fuInt UpdateMaxFPS=0, fuInt RenderMaxFPS=0)=0;

	/// @brief 返回线程模式
	virtual F2DENGTHREADMODE GetCurrentThreadMode()=0;

	// --- 消息控制 ---
	/// @brief     发送消息
	/// @note      线程安全
	/// @param[in] Msg     消息
	/// @param[in] pMemObj 可空，消息附加的内存数据
	virtual fResult SendMsg(const f2dMsg& Msg, f2dInterface* pMemObj = NULL)=0;
	
	/// @brief     发送消息
	/// @note      线程安全
	/// @param[in] Type    消息类型
	/// @param[in] Param1  参数1
	/// @param[in] Param2  参数2
	/// @param[in] Param3  参数3
	/// @param[in] Param4  参数4
	/// @param[in] pMemObj 可空，消息附加的内存数据
	virtual fResult SendMsg(F2DMSGTYPE Type, fuLong Param1 = 0, fuLong Param2 = 0, fuLong Param3 = 0, fuLong Param4 = 0, f2dInterface* pMemObj = NULL)=0;

	/// @brief     调用委托方法
	/// @param[in] pDelegate 委托事件对象
	virtual fResult InvokeDelegate(f2dMainThreadDelegate* pDelegate)=0;
	
	/// @brief     调用委托方法并等待
	/// @param[in] pDelegate 委托事件对象
	virtual fResult InvokeDelegateAndWait(f2dMainThreadDelegate* pDelegate)=0;

	// --- 扩展功能 ---
	/// @brief     返回CPU信息
	/// @param[in] Info 输出的CPU信息
	virtual void GetCPUInfo(f2dCPUInfo& Info)=0;
};
/// @}
