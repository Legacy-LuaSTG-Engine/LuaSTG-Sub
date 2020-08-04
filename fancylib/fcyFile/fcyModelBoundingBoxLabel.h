////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelBoundingBoxLabel.h
/// @brief fcy模型包围盒数据标签定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyModelLabel.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy模型包围盒数据标签
////////////////////////////////////////////////////////////////////////////////
class fcyModelBoundingBoxLabel :
	public fcyModelLabel
{
public:
	/// @brief 标签名
	static const fChar LABELNAME[8];
protected:
	// 包围盒信息
	fcyVec3 m_Min;   ///< @brief 最小坐标
	fcyVec3 m_Max;   ///< @brief 最大坐标
public:
	/// @brief 返回最小坐标
	const fcyVec3& GetMin()const { return m_Min; }
	/// @brief 返回最大坐标
	const fcyVec3& GetMax()const { return m_Max; }
	/// @brief 设置最小坐标
	void SetMin(const fcyVec3& Value) { m_Min = Value; }
	/// @brief 设置最大坐标
	void SetMax(const fcyVec3& Value) { m_Max = Value; }
public: // 接口实现
	void ReadData(fcyStream* pStream);
	void WriteData(fcyStream* pStream);
public:
	fcyModelBoundingBoxLabel()
		: fcyModelLabel(FMM_MAKE_LABELNAME8(LABELNAME)) {}
protected:
	~fcyModelBoundingBoxLabel() {}
};

/// @}
