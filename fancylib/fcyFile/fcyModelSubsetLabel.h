////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelSubsetLabel.h
/// @brief fcy模型子集数据标签定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyModelLabel.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy模型子集数据标签
////////////////////////////////////////////////////////////////////////////////
class fcyModelSubsetLabel :
	public fcyModelLabel
{
public:
	/// @brief 标签名
	static const fChar LABELNAME[8];

	/// @brief 图元类型
	enum PRIMTYPE
	{
		PRIMTYPE_NULL          = 0,  ///< @brief 无效值
		PRIMTYPE_POINTLIST     = 1,  ///< @brief 点集
		PRIMTYPE_LINELIST      = 2,  ///< @brief 线集
		PRIMTYPE_LINESTRIP     = 3,  ///< @brief 线带集
		PRIMTYPE_TRIANGLELIST  = 4,  ///< @brief 三角形集
		PRIMTYPE_TRIANGLESTRIP = 5,  ///< @brief 三角形带
		PRIMTYPE_TRIANGLEFAN   = 6   ///< @brief 三角形扇
	};
protected:
	// 子集数据
	std::wstring m_SubsetName;      ///< @brief 子集名称，在模型数据中不可重复
	std::wstring m_MaterialName;    ///< @brief 材质名称
	PRIMTYPE m_PrimitiveType;       ///< @brief 图元类型
	fuInt m_StartIndex;             ///< @brief 索引开始下标
	fuInt m_PrimitiveCount;         ///< @brief 图元数量
public:
	fcStrW GetSubsetName()const { return m_SubsetName.c_str(); }
	fcStrW GetMaterialName()const { return m_MaterialName.c_str(); }
	PRIMTYPE GetPrimitiveType()const { return m_PrimitiveType; }
	fuInt GetStartIndex()const { return m_StartIndex; }
	fuInt GetPrimitiveCount()const { return m_PrimitiveCount; }

	void SetSubsetName(const std::wstring& Str) { m_SubsetName = Str; }
	void SetMaterialName(const std::wstring& Str) { m_MaterialName = Str; }
	void SetPrimitiveType(PRIMTYPE Type) { m_PrimitiveType = Type; }
	void SetStartIndex(fuInt Value) { m_StartIndex = Value; }
	void SetPrimitiveCount(fuInt Count) { m_PrimitiveCount = Count; }
public: // 接口实现
	void ReadData(fcyStream* pStream);
	void WriteData(fcyStream* pStream);
public:
	fcyModelSubsetLabel()
		: fcyModelLabel(FMM_MAKE_LABELNAME8(LABELNAME)) {}
protected:
	~fcyModelSubsetLabel() {}
};
/// @}
