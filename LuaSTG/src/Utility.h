/// @file Utility.h
/// @brief 实用工具
#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	///离开作用域自动执行委托的函数
	class Scope
	{
	private:
		std::function<void()> m_WhatToDo;
	private:
		Scope& operator=(const Scope&);
		Scope(const Scope&);
	public:
		Scope(std::function<void()> exitJob)
			: m_WhatToDo(exitJob) {}
		~Scope()
		{
			m_WhatToDo();
		}
	};

	///计时域，离开作用域自动计算流逝的时间
	class TimerScope
	{
	private:
		fcyStopWatch m_StopWatch;
		float& m_Out;
	public:
		TimerScope(float& Out)
			: m_Out(Out)
		{
		}
		~TimerScope()
		{
			m_Out = static_cast<float>(m_StopWatch.GetElapsed());
		}
	};
	
	// com组件库域，自动初始化com组件库和离开作用域时自动卸载com组件库，以函数调用时获取com组件库初始化成功与否
	class CoInitializeScope {
	private:
		bool m_Status = false;
	public:
		bool operator ()()const { return m_Status; }
		CoInitializeScope() { m_Status = SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)); }
		~CoInitializeScope() { if (m_Status) CoUninitialize(); }
	};
}
