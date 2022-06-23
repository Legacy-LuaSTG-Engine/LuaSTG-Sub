#pragma once

// 数学常量 pi
constexpr double L_PI        = 3.1415926535897932384626433832795;
constexpr float  L_PI_F      = 3.1415926535897932384626433832795f;
// pi 的一半
constexpr double L_PI_HALF   = L_PI * 0.5;
constexpr float  L_PI_HALF_F = L_PI_F * 0.5f;
// pi 的两倍 tau
constexpr double L_TAU       = L_PI * 2.0;
constexpr float  L_TAU_F     = L_PI_F * 2.0f;

// 弧度到角度
constexpr double L_RAD_TO_DEG   = 180.0 / L_PI;
constexpr float  L_RAD_TO_DEG_F = 180.0f / L_PI_F;
// 角度到弧度
constexpr double L_DEG_TO_RAD   = L_PI / 180.0;
constexpr float  L_DEG_TO_RAD_F = L_PI_F / 180.0f;
