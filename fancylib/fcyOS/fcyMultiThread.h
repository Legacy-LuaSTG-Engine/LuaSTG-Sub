////////////////////////////////////////////////////////////////////////////////
/// @file  fcyMultiThread.h
/// @brief fancy多线程支持
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../fcyType.h"
#include "../fcyException.h"

/// @addtogroup fancy库底层支持
/// @brief 实现一系列涉及系统内核操作的函数

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy线程基类
/// @note  通过继承该类并覆写ThreadJob来实现多线程
////////////////////////////////////////////////////////////////////////////////
class fcyBaseThread {
private:
    fHandle m_hThread = 0;  ///< @brief 内部的线程句柄
    uint32_t m_hThreadID = 0; ///< @brief 线程ID
private:
    static uint32_t __stdcall ThreadFunction(void* p);

protected:
    /// @brief  [保护] 线程工作函数
    /// @note   使用多线程时必须覆写改函数
    /// @return 返回线程执行状态，参见MSDN
    virtual uint32_t ThreadJob() = 0;

public:
    /// @brief   返回内部线程句柄
    /// @warning 请不要手动关闭返回的内核对象
    /// @return  线程句柄
    fHandle GetHandle();
    
    /// @brief  恢复线程执行
    /// @return true=成功，false=失败
    bool Resume();
    
    /// @brief  暂停线程执行
    /// @return true=成功，false=失败
    bool Suspend();
    
    /// @brief     等待线程执行完毕
    /// @param[in] TimeLimited 等待时间，置为-1表示无限等待
    /// @return    true=成功，false=超时或其他Win32错误
    bool Wait(int32_t TimeLimited = -1);
    
    /// @brief     终止线程
    /// @param[in] ExitCode 线程返回值，默认为-1
    /// @return    true=成功，false=Win32错误
    bool Terminate(int32_t ExitCode = -1);
    
    /// @brief 获得线程返回值
    uint32_t GetExitCode();

protected:
    /// @brief     构造函数
    /// @param[in] PauseThread 设置为true表示创建后暂停线程执行，否则立即执行线程
    fcyBaseThread(bool PauseThread = true);
    
    ~fcyBaseThread();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy临界区
/// @note  临界区，用来快速对一段代码进行加锁
////////////////////////////////////////////////////////////////////////////////
class fcyCriticalSection {
private:
    uint64_t m_Section[8] = {}; // CRITICAL_SECTION
public:
    /// @brief 锁定临界区
    void Lock();
    
    /// @brief  试图锁定临界区
    /// @return true=成功，false=失败
    bool TryLock();
    
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
class fcyEvent {
private:
    fHandle m_hEvent = 0;
public:
    /// @brief   返回内部事件句柄
    /// @warning 请不要手动关闭返回的内核对象
    /// @return  事件句柄
    fHandle GetHandle();
    
    /// @brief  标记事件
    /// @return true=成功，false=Win32错误
    bool Set();
    
    /// @brief  取消事件标记
    /// @return true=成功，false=Win32错误
    bool Reset();
    
    /// @brief  事件脉冲
    /// @return true=成功，flase=Win32错误
    bool Pulse();
    
    /// @brief     等待事件
    /// @param[in] TimeLimited 等待事件，-1表示无限等待
    /// @return    true=成功，flase=超时或Win32错误
    bool Wait(int32_t TimeLimited = -1);

public:
    /// @brief 构造函数
    fcyEvent(bool AutoReset = false, bool InitalState = false);
    
    ~fcyEvent();
};
