////////////////////////////////////////////////////////////////////////////////
/// @file  fcyHash.h
/// @brief fancy哈希
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyType.h"
#include <functional>

/// @addtogroup fancy杂项
/// @{

/// @brief fcyHash支持
namespace fcyHash
{
	/// @brief     SuperFastHash算法
	/// @param[in] pData   原始数据
	/// @param[in] DataLen 原始数据长度
	fuInt SuperFastHash(fcData pData, fuInt DataLen);

	/// @brief SuperFastHash算法模板
	template<typename T>
	fuInt SuperFastHash(const T& Data)
	{
		return SuperFastHash((fcData)&Data, sizeof(T));
	}

	/// @brief HASH值组合模板
	template <typename T>
	inline void HashCombine(fuInt& seed, const T & v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
};

/// @}
