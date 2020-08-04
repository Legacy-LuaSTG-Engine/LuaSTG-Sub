////////////////////////////////////////////////////////////////////////////////
/// @file  fcyRandom.h
/// @brief fancy随机数
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyType.h"

/// @addtogroup fancy杂项
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief WELL512随机数算法
/// @note  摘自《游戏编程精粹 7》
////////////////////////////////////////////////////////////////////////////////
class fcyRandomWELL512
{
private:
	fuInt m_State[16];   ///< @brief 内部状态机
	fuInt m_Index;       ///< @brief 下标
	fuInt m_RSeed;       ///< @brief 随机数种子
public:
	/// @brief 获得随机数种子
	fuInt GetRandSeed();
	/// @brief     设置随机数种子
	/// @param[in] Seed 随机数种子
	void SetSeed(fuInt Seed);
	/// @brief 产生无符号整数
	fuInt GetRandUInt();
	/// @brief     产生[0, Max]之间的无符号整数
	/// @param[in] Max 最大数
	fuInt GetRandUInt(fuInt Max);
	/// @brief 产生[0, 1]之间的随机小数
	fFloat GetRandFloat();
	/// @brief 产生[MinBound, MaxBound]之间的随机小数
	/// @param[in] MinBound 下界
	/// @param[in] MaxBound 上界
	fFloat GetRandFloat(fFloat MinBound, fFloat MaxBound);
public:
	/// @brief 默认构造函数
	/// @note  调用GetTickCount()进行初始化
	fcyRandomWELL512();
	/// @brief 指定种子初始化随机数发生器
	fcyRandomWELL512(fuInt Seed);
	~fcyRandomWELL512();
};
/// @}
