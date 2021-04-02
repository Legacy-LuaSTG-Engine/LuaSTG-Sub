#pragma once

#include <cstdint>

namespace LuaSTGPlus {
    // 计时域，离开作用域自动计算流逝的时间
    class TimerScope {
    private:
        int64_t _freq;
        int64_t _time;
        float& _out;
    public:
        float operator()() const;
        explicit TimerScope(float& Out);
        ~TimerScope();
    };
    
    // 自动配对调用 CoInitializeEx 和 CoUninitialize
    class CoInitializeScope {
    private:
        bool _result = false;
    public:
        inline bool operator()() const { return _result; }
        CoInitializeScope();
        ~CoInitializeScope();
    };
}
