////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelVertexLabel.h
/// @brief fcy模型顶点数据标签定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyModelLabel.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fcy模型顶点数据标签
////////////////////////////////////////////////////////////////////////////////
class fcyModelVertexLabel :
	public fcyModelLabel
{
public:
	/// @brief 标签名
	static const fChar LABELNAME[8];

	/// @brief FMM顶点元素类型
	enum ELEMENTTYPE 
	{
		ELEMENTTYPE_FLOAT,   ///< @brief 4字节浮点
		ELEMENTTYPE_FLOAT2,  ///< @brief 2*4字节浮点
		ELEMENTTYPE_FLOAT3,  ///< @brief 3*4字节浮点
		ELEMENTTYPE_FLOAT4,  ///< @brief 4*4字节浮点
		ELEMENTTYPE_COLOR,   ///< @brief 4*1字节BGRA颜色分量
		ELEMENTTYPE_UBYTE4,  ///< @brief 4*1字节
		ELEMENTTYPE_SHORT2,  ///< @brief 2*2有符号短整数
		ELEMENTTYPE_SHORT4   ///< @brief 4*2有符号短整数
	};

	/// @brief FMM顶点元素
	struct VertexElement
	{
		ELEMENTTYPE Type;  ///< @brief 描述元素的数据格式，定义如下
		fuLong Usage;      ///< @brief 描述元素的用途
		fByte UsageIndex;  ///< @brief 描述元素的用途索引
	};
protected:
	// 顶点声明数据
	std::vector<VertexElement> m_VertElementList;
	
	// 顶点数据
	std::vector<fByte> m_VertexData;
public:
	/// @brief     依据顶点声明计算顶点大小
	fuInt CalcuVertexSize()const;

	/// @brief     追加一个顶点
	/// @param[in] pData      顶点数据
	/// @param[in] VertexSize 顶点大小
	void PushVertex(void* pData, fuInt VertexSize);

	/// @brief     追加一个顶点(模板)
	template<typename T>
	void PushVertex(T* Vertex)
	{
		PushVertex(Vertex, sizeof(T));
	}

	/// @brief     返回Vertex数据指针
	/// @warning   在重新分配大小后应该重新获得指针
	void* GetVertexDataPointer() { return (m_VertexData.size() == 0) ? NULL : m_VertexData.data(); }

	/// @brief     返回顶点缓冲大小
	fuInt GetVertexDataSize()const { return m_VertexData.size(); }

	/// @brief     重新分配顶点数据大小
	/// @note      保留原始数据
	/// @param[in] NewSize 字节，新的顶点缓冲大小
	void ResizeVertex(fuInt NewSize) { m_VertexData.resize(NewSize); }
	
	/// @brief     追加一个顶点声明
	/// @param[in] Element 顶点元素
	void PushVertexElement(const VertexElement& Element) { m_VertElementList.push_back(Element); }

	/// @brief     返回元素
	/// @param[in] Index 元素索引
	VertexElement& GetVertexElement(fuInt Index) { return m_VertElementList[Index]; }

	/// @brief     返回顶点声明元素数量
	fuInt GetVertexElementSize()const { return m_VertElementList.size(); }

	/// @brief     重新分配数据大小
	/// @param[in] 元素数量
	void ResizeElement(fuInt Count) { m_VertElementList.resize(Count); }
public: // 接口实现
	void ReadData(fcyStream* pStream);
	void WriteData(fcyStream* pStream);
public:
	fcyModelVertexLabel()
		: fcyModelLabel(FMM_MAKE_LABELNAME8(LABELNAME)) {}
protected:
	~fcyModelVertexLabel() {}
};
/// @}
