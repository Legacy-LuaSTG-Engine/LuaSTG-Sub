////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelIndexLabel.h
/// @brief fcy模型索引数据标签定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyModelLabel.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy模型索引数据标签
////////////////////////////////////////////////////////////////////////////////
class fcyModelIndexLabel :
	public fcyModelLabel
{
public:
	/// @brief 标签名
	static const fChar LABELNAME[8];
protected:
	// 索引数据
	std::vector<fuInt> m_IndexData;
public:
	/// @brief 检查索引格式
	fBool IsIndex32();

	/// @brief     追加一个索引
	/// @param[in] Index 索引
	void Push(fuInt Index) { m_IndexData.push_back(Index); }

	/// @brief 获得索引数据
	fuInt* GetDataPointer() { return m_IndexData.data(); }

	/// @brief     返回元素
	/// @param[in] Index 元素索引
	fuInt& GetElement(fuInt Index) { return m_IndexData[Index]; }

	/// @brief     返回顶点声明元素数量
	fuInt GetSize()const { return m_IndexData.size(); }

	/// @brief     重新分配数据大小
	/// @param[in] 元素数量
	void Resize(fuInt Count) { m_IndexData.resize(Count); }
public: // 接口实现
	void ReadData(fcyStream* pStream);
	void WriteData(fcyStream* pStream);
public:
	fcyModelIndexLabel()
		: fcyModelLabel(FMM_MAKE_LABELNAME8(LABELNAME)) {}
protected:
	~fcyModelIndexLabel() {}
};
/// @}
