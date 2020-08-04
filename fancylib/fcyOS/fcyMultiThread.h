////////////////////////////////////////////////////////////////////////////////
/// @file  fcyMultiThread.h
/// @brief fancy多线程支持
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyType.h"
#include "../fcyException.h"

#include <Windows.h>

/// @addtogroup fancy库底层支持
/// @brief 实现一系列涉及系统内核操作的函数
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy线程基类
/// @note  通过继承该类并覆写ThreadJob来实现多线程
////////////////////////////////////////////////////////////////////////////////
class fcyBaseThread
{
private:
	HANDLE m_hThread;  ///< @brief 内部的线程句柄
	DWORD m_hThreadID; ///< @brief 线程ID
private:
	/// @brief     [私有] 执行线程的包装函数
	/// @param[in] p 即线程类指针
	/// @return    返回线程执行状态，参见MSDN
	static DWORD WINAPI execute(void* p);
protected:
	/// @brief  [保护] 线程工作函数
	/// @note   使用多线程时必须覆写改函数
	/// @return 返回线程执行状态，参见MSDN
	virtual fuInt ThreadJob()=0;
public:
	/// @brief   返回内部线程句柄
	/// @warning 请不要手动关闭返回的内核对象
	/// @return  线程句柄
	HANDLE GetHandle();

	/// @brief  恢复线程执行
	/// @return true=成功，false=失败
	fBool Resume();
	
	/// @brief  暂停线程执行
	/// @return true=成功，false=失败
	fBool Suspend();

	/// @brief     等待线程执行完毕
	/// @param[in] TimeLimited 等待时间，置为-1表示无限等待
	/// @return    true=成功，false=超时或其他Win32错误
	fBool Wait(fInt TimeLimited=-1);

	/// @brief     终止线程
	/// @param[in] ExitCode 线程返回值，默认为-1
	/// @return    true=成功，false=Win32错误
	fBool Terminate(fInt ExitCode=-1);

	/// @brief 获得线程返回值
	fuInt GetExitCode();
protected:
	/// @brief     构造函数
	/// @param[in] PauseThread 设置为true表示创建后暂停线程执行，否则立即执行线程
	fcyBaseThread(fBool PauseThread = true);
	~fcyBaseThread();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy临界区
/// @note  临界区，用来快速对一段代码进行加锁
////////////////////////////////////////////////////////////////////////////////
class fcyCriticalSection
{
private:
	CRITICAL_SECTION m_Section; ///< @brief 内置临界区
public:
	/// @brief 锁定临界区
	void Lock();

	/// @brief  试图锁定临界区
	/// @return true=成功，false=失败
	fBool TryLock();

	/// @brief 解锁临界区
	/// @note  Lock以及TryLock成功后必须使用该函数标注解锁
	void UnLock();
public:
	fcyCriticalSection();
	~fcyCriticalSection();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy事件
/// @note  通过事件对多线程操作进行同步
////////////////////////////////////////////////////////////////////////////////
class fcyEvent
{
private:
	HANDLE m_hEvent; ///< @brief 内置事件句柄
public:
	/// @brief   返回内部事件句柄
	/// @warning 请不要手动关闭返回的内核对象
	/// @return  事件句柄
	HANDLE GetHandle();

	/// @brief  标记事件
	/// @return true=成功，false=Win32错误
	fBool Set();

	/// @brief  取消事件标记
	/// @return true=成功，false=Win32错误
	fBool Reset();

	/// @brief  事件脉冲
	/// @return true=成功，flase=Win32错误
	fBool Pulse();

	/// @brief     等待事件
	/// @param[in] TimeLimited 等待事件，-1表示无限等待
	/// @return    true=成功，flase=超时或Win32错误
	fBool Wait(fInt TimeLimited=-1);
public:
	/// @brief 构造函数
	fcyEvent(fBool AutoReset=false, fBool InitalState=false);
	~fcyEvent();
};
/// @}