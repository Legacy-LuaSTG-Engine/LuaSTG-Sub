#include "Utility/Utility.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <combaseapi.h>

namespace LuaSTGPlus
{
    // 离开作用域自动执行委托的函数
    class Scope
    {
    private:
        std::function<void()> m_WhatToDo;
    public:
        explicit Scope(std::function<void()> exitJob) : m_WhatToDo(std::move(exitJob)) {}
        ~Scope() { m_WhatToDo(); }
    };
    
    float TimerScope::operator()() const
    {
        LARGE_INTEGER time = {};
        ::QueryPerformanceCounter(&time);
        return float(double(time.QuadPart - _time) / double(_freq));
    }
    TimerScope::TimerScope(float& inout) : _freq(0), _time(0), _out(inout)
    {
        LARGE_INTEGER freq = {};
        ::QueryPerformanceFrequency(&freq);
        _freq = freq.QuadPart;
        LARGE_INTEGER time = {};
        ::QueryPerformanceCounter(&time);
        _time = time.QuadPart;
        _out = 0.0f; // clean
    }
    TimerScope::~TimerScope()
    {
        LARGE_INTEGER time = {};
        ::QueryPerformanceCounter(&time);
        _out = float(double(time.QuadPart - _time) / double(_freq));
    }
    
    CoInitializeScope::CoInitializeScope()
    {
        _result = SUCCEEDED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    }
    CoInitializeScope::~CoInitializeScope()
    {
        if (_result)
        {
            ::CoUninitialize();
        }
    }
}
