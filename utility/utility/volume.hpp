#pragma once
#include <cmath>

namespace utility::volume
{
    // 分贝转电平，电平以1V为基准
    inline double dB_to_V(double dB)
    {
        return std::pow(10.0, dB / 20.0);
    }
    
    // 电平转分贝，电平以1V为基准
    inline double V_to_dB(double v)
    {
        return 20.0 * std::log10(v);
    }
    
    // 音量的线性转对数
    // 输入归一化线性音量取值范围，取值范围[0.0, 1.0]
    // 返回归一化分贝衰减模型对数音量，范围[0.0, 1.0]
    inline double linear_to_log(double v)
    {
        if (v <= 0.0) return 0.0;
        v = (v < 1.0) ? v : 1.0;
        const double dB = V_to_dB(v);
        const double rate = (dB + 100.0) / 100.0;
        return rate;
    }
    
    // 音量的对数转线性
    // 输入归一化分贝衰减模型对数音量，取值范围[0.0, 1.0]
    // 返回归一化线性音量，范围[0.0, 1.0]
    inline double log_to_linear(double d)
    {
        if (d <= 0.0) return 0.0;
        d = (d < 1.0) ? d : 1.0;
        const double dB = 100.0 * d - 100.0;
        const double rate = dB_to_V(dB);
        return rate;
    }
}
