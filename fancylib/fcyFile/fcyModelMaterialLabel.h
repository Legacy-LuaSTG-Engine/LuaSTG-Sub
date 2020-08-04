////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelMaterialLabel.h
/// @brief fcy模型材质数据标签定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyModelLabel.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy模型材质数据标签
////////////////////////////////////////////////////////////////////////////////
class fcyModelMaterialLabel :
	public fcyModelLabel
{
public:
	/// @brief 标签名
	static const fChar LABELNAME[8];
protected:
	// 材质数据
	std::wstring m_MatName;    ///< @brief 材质名称
	std::wstring m_MatXMLData; ///< @brief 材质数据
public:
	/// @brief 返回材质名称
	fcStrW GetMaterialName()const { return m_MatName.c_str(); }
	
	/// @brief 返回材质XML数据
	fcStrW GetMaterialXMLData()const { return m_MatXMLData.c_str(); }

	/// @brief 设置材质名称
	void SetMaterialName(const std::wstring& Name) { m_MatName = Name; }

	/// @brief 设置XML数据
	void SetMaterialXMLData(const std::wstring XMLData) { m_MatXMLData = XMLData; }
public: // 接口实现
	void ReadData(fcyStream* pStream);
	void WriteData(fcyStream* pStream);
public:
	fcyModelMaterialLabel()
		: fcyModelLabel(FMM_MAKE_LABELNAME8(LABELNAME)) {}
protected:
	~fcyModelMaterialLabel() {}
};

/// @}
