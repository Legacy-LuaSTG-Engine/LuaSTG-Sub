////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelMeshFile.h
/// @brief fcy模型网格
/// @note  支持了fancy模型网格文件的读写
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyIO/fcyStream.h"
#include "../fcyIO/fcyBinaryHelper.h"
#include "../fcyMisc/fcyStringHelper.h"

#include <vector>

#include "fcyModelVertexLabel.h"
#include "fcyModelIndexLabel.h"
#include "fcyModelMaterialLabel.h"
#include "fcyModelSubsetLabel.h"
#include "fcyModelBoundingBoxLabel.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy网格模型文件
////////////////////////////////////////////////////////////////////////////////
class fcyModelMeshFile
{
public:
	static const fuInt Version = 100;  ///< @brief 版本号1.00
protected:
	std::wstring m_Author;             ///< @brief 作者
	std::wstring m_Description;        ///< @brief 模型描述

	std::vector< fcyRefPointer<fcyModelLabel> > m_Labels;   ///< @brief 标签数据域
public:
	/// @brief 返回作者
	fcStrW GetAuthor()const { return m_Author.c_str(); }
	/// @brief 返回描述
	fcStrW GetDescription()const { return m_Description.c_str(); }
	/// @brief 设置作者
	void SetAuthor(const std::wstring& Str) { m_Author = Str; }
	/// @brief 设置描述
	void SetDescription(const std::wstring& Str) { m_Description = Str; }
	
	/// @brief 返回标签列表
	std::vector< fcyRefPointer<fcyModelLabel> >& GetLabelList() { return m_Labels; }
	
	/// @brief 返回第一个标签
	fcyRefPointer<fcyModelLabel> FindLabel(fuLong LabelName);

	/// @brief 加载数据
	void Load(fcyStream* pStream, fBool IgnoreUnsupportLabel = true);

	/// @brief 保存数据
	void Save(fcyStream* pStream);

	/// @brief 导出到fancy2D模型
	// void Export();
public:
	fcyModelMeshFile() {}
	/// @brief 浅拷贝
	fcyModelMeshFile(const fcyModelMeshFile& Right)
		: m_Author(Right.m_Author), m_Description(Right.m_Description),
		m_Labels(Right.m_Labels) {}
	~fcyModelMeshFile() {}
};

/// @}
