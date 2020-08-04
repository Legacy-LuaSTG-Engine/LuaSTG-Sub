////////////////////////////////////////////////////////////////////////////////
/// @file  fcyModelMeshExporter.h
/// @brief fcy模型网格导出
/// @note  实现从fmm导出到f2d模型数据
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyModelMeshFile.h"

/// @addtogroup fancy库文件支持
/// @brief 提供部分格式文件的读写支持
/// @{

struct f2dRenderDevice;
struct f2dMeshData;

/// @brief 网格导出
namespace fcyModelMeshExporter
{
	////////////////////////////////////////////////////////////////////////////////
	/// @brief fancy网格导出信息
	////////////////////////////////////////////////////////////////////////////////
	class ModelMeshInfo
	{
		friend ModelMeshInfo ExportMesh(fcyModelMeshFile& Mesh, f2dRenderDevice* pDev, f2dMeshData** pOut);
	private:
		std::unordered_map<std::wstring, fuInt> m_SubsetNameToId;
		std::unordered_map<fuInt, fuInt> m_IdToSubsetIndex;
		std::vector<fcyRefPointer<fcyModelSubsetLabel>> m_SubsetList;
		std::unordered_map<std::wstring, fcyRefPointer<fcyModelMaterialLabel>> m_MaterialList;
	public:
		/// @brief 返回子集数量
		fuInt GetSubsetCount()const { return m_SubsetList.size(); }

		/// @brief 返回子集
		/// @note  返回弱引用
		fcyModelSubsetLabel* GetSubset(fuInt Index) { return m_SubsetList[Index]; }
		
		/// @brief 快速查询材质
		/// @note  返回弱引用
		fcyModelMaterialLabel* GetMaterial(const std::wstring& Str) { return m_MaterialList[Str]; }

		/// @brief 查询子集ID
		fuInt GetSubsetID(const std::wstring& Str)const
		{
			std::unordered_map<std::wstring, fuInt>::const_iterator i = m_SubsetNameToId.find(Str);
			if(i == m_SubsetNameToId.end())
				throw fcyException("ModelMeshInfo::GetSubsetID", "Object not exist.");
			else
				return i->second;
		}

		/// @brief ID倒查子集Index
		fuInt GetIndex(fuInt ID)const
		{
			std::unordered_map<fuInt, fuInt>::const_iterator i = m_IdToSubsetIndex.find(ID);
			if(i == m_IdToSubsetIndex.end())
				throw fcyException("ModelMeshInfo::GetSubsetID", "Object not exist.");
			else
				return i->second;
		}
	public:
		ModelMeshInfo() {}
		ModelMeshInfo(const ModelMeshInfo& Org)
			: m_SubsetNameToId(Org.m_SubsetNameToId), m_IdToSubsetIndex(Org.m_IdToSubsetIndex),
			m_SubsetList(Org.m_SubsetList), m_MaterialList(Org.m_MaterialList) {}
	};


	////////////////////////////////////////////////////////////////////////////////
	/// @brief 导出模型到f2d
	////////////////////////////////////////////////////////////////////////////////
	ModelMeshInfo ExportMesh(fcyModelMeshFile& Mesh, f2dRenderDevice* pDev, f2dMeshData** pOut);
};

/// @}
