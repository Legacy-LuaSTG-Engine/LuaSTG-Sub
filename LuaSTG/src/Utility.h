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
	class CoScope {
	private:
		bool m_Status = false;
	public:
		bool operator ()()const { return m_Status; }
		CoScope() { m_Status = SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)); }
		~CoScope() { if (m_Status) CoUninitialize(); }
	};

	//字符串格式化
	//param[in] Format 字符串格式，不支持精度
	std::string StringFormat(const char* Format, ...)noexcept;

	//字符串格式化 va_list版本
	std::string StringFormatV(const char* Format, va_list vaptr)noexcept;

	//字符串格式化 宽字符
	//param[in] Format 字符串格式，不支持精度
	std::wstring StringFormat(const wchar_t* Format, ...)noexcept;

	//字符串格式化 宽字符、va_list版本
	std::wstring StringFormatV(const wchar_t* Format, va_list vaptr)noexcept;

	//RC4加解密实现
	class RC4
	{
	private:
		uint8_t S[256];
	public:
		void operator()(const uint8_t* input, size_t inputlen, uint8_t* output)
        {
            uint8_t Scpy[256];
            memcpy(Scpy, S, sizeof(S));

            for (size_t i = 0, j = 0; i < inputlen; i++)
            {
                // S盒置换
                size_t i2 = (i + 1) % 256;
                j = (j + Scpy[i2]) % 256;
                std::swap(Scpy[i2], Scpy[j]);
                uint8_t n = Scpy[(Scpy[i2] + Scpy[j]) % 256];

                // 加解密
                *(output + i) = *(input + i) ^ n;
            }
        }
	public:
		RC4(const uint8_t* password, size_t len)
		{
			len = (len < 256U) ? len : 256U;
			// 初始化S盒
			for (int i = 0; i < 256; ++i)
				S[i] = i;

			// S盒初始置换
			for (size_t i = 0, j = 0; i < 256; i++)
			{
				j = (j + S[i] + password[i % len]) % 256;
				std::swap(S[i], S[j]);
			}
		}
	};

	//获取系统本地应用数据文件夹
	std::wstring GetLocalAppDataPath()noexcept;

	//获取系统漫游应用数据文件夹
	std::wstring GetRoamingAppDataPath()noexcept;
}
