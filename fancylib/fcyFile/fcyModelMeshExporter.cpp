#include "fcyModelMeshExporter.h"

#include "../../fancy2D/f2d.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyModelMeshExporter::ModelMeshInfo fcyModelMeshExporter::ExportMesh(fcyModelMeshFile& Mesh, f2dRenderDevice* pDev, f2dMeshData** pOut)
{
	if(!pDev || !pOut)
		throw fcyException("fcyModelMeshExporter::ExportMesh", "Invalid Pointer.");
	*pOut = NULL;

	// 获得关键标签
	fcyRefPointer<fcyModelLabel> tVertexLabel = Mesh.FindLabel(FMM_MAKE_LABELNAME8(fcyModelVertexLabel::LABELNAME));
	fcyRefPointer<fcyModelLabel> tIndexLabel = Mesh.FindLabel(FMM_MAKE_LABELNAME8(fcyModelIndexLabel::LABELNAME));
	
	// 检查关键标签是否存在
	if(!tVertexLabel || !tIndexLabel)
		throw fcyException("fcyModelMeshExporter::ExportMesh", "Data is not correct.");

	// 创建顶点声明
	vector<f2dVertexElement> tVertexElementList;
	tVertexElementList.resize(((fcyModelVertexLabel*)*tVertexLabel)->GetVertexElementSize());
	for(fuInt i = 0; i<tVertexElementList.size(); ++i)
	{
		fcyModelVertexLabel::VertexElement& tElement = ((fcyModelVertexLabel*)*tVertexLabel)->GetVertexElement(i);
		
		tVertexElementList[i].Type = (F2DVDTYPE)tElement.Type;
		tVertexElementList[i].UsageIndex = tElement.UsageIndex;

		fuLong tUsage = tElement.Usage;

		if(tUsage == FMM_MAKE_LABELNAME8("POSITION"))
			tVertexElementList[i].Usage = F2DVDUSAGE_POSITION;
		else if(tUsage == FMM_MAKE_LABELNAME8("WEIGHT\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_BLENDWEIGHT;
		else if(tUsage == FMM_MAKE_LABELNAME8("INDICES\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_BLENDINDICES;
		else if(tUsage == FMM_MAKE_LABELNAME8("NORMAL\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_NORMAL;
		else if(tUsage == FMM_MAKE_LABELNAME8("PSIZE\0\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_PSIZE;
		else if(tUsage == FMM_MAKE_LABELNAME8("TEXCOORD"))
			tVertexElementList[i].Usage = F2DVDUSAGE_TEXCOORD;
		else if(tUsage == FMM_MAKE_LABELNAME8("TANGENT\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_TANGENT;
		else if(tUsage == FMM_MAKE_LABELNAME8("BINORMAL"))
			tVertexElementList[i].Usage = F2DVDUSAGE_BINORMAL;
		else if(tUsage == FMM_MAKE_LABELNAME8("TESSFACT"))
			tVertexElementList[i].Usage = F2DVDUSAGE_TESSFACTOR;
		else if(tUsage == FMM_MAKE_LABELNAME8("POST\0\0\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_POSITIONT;
		else if(tUsage == FMM_MAKE_LABELNAME8("COLOR\0\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_COLOR;
		else if(tUsage == FMM_MAKE_LABELNAME8("FOG\0\0\0\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_FOG;
		else if(tUsage == FMM_MAKE_LABELNAME8("DEPTH\0\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_DEPTH;
		else if(tUsage == FMM_MAKE_LABELNAME8("SAMPLE\0\0"))
			tVertexElementList[i].Usage = F2DVDUSAGE_SAMPLE;
		else 
		{
			throw fcyException("fcyModelMeshExporter::ExportMesh", "Unknwon Vertex Type.");
		}
	}

	// 检查格式并创建模型
	fBool tInt32Index = ((fcyModelIndexLabel*)*tIndexLabel)->IsIndex32();
	fuInt tPerVertSize = ((fcyModelVertexLabel*)*tVertexLabel)->CalcuVertexSize();
	fResult tFR = pDev->CreateMeshData(
		tVertexElementList.data(), 
		tVertexElementList.size(), 
		((fcyModelVertexLabel*)*tVertexLabel)->GetVertexDataSize() / tPerVertSize, 
		((fcyModelIndexLabel*)*tIndexLabel)->GetSize(),
		tInt32Index,
		pOut);
	if(FCYFAILED(tFR))
	{
		throw fcyException("fcyModelMeshExporter::ExportMesh", "CreateMeshData Failed.");
	}

	// 填充数据
	fByte* pVert = NULL;
	(*pOut)->LockVertexData(0, ((fcyModelVertexLabel*)*tVertexLabel)->GetVertexDataSize() / tPerVertSize, (void**)&pVert);
	memcpy(pVert, ((fcyModelVertexLabel*)*tVertexLabel)->GetVertexDataPointer(), ((fcyModelVertexLabel*)*tVertexLabel)->GetVertexDataSize());
	(*pOut)->UnlockVertexData();

	fByte* pIndex = NULL;
	(*pOut)->LockIndexData(0, ((fcyModelIndexLabel*)*tIndexLabel)->GetSize(), (void**)&pIndex);
	for(fuInt i = 0; i<((fcyModelIndexLabel*)*tIndexLabel)->GetSize(); ++i)
	{
		if(tInt32Index)
		{
			*((fuInt*)pIndex) = ((fcyModelIndexLabel*)*tIndexLabel)->GetElement(i);
			pIndex += sizeof(fuInt);
		}
		else
		{
			*((fuShort*)pIndex) = ((fcyModelIndexLabel*)*tIndexLabel)->GetElement(i);
			pIndex += sizeof(fuShort);
		}
	}
	(*pOut)->UnlockIndexData();

	// 遍历并拷贝子集
	fuInt tID = 0;
	ModelMeshInfo tInfo;
	for(fuInt i = 0; i<Mesh.GetLabelList().size(); ++i)
	{
		fcyRefPointer<fcyModelLabel> tSubsetLabel = Mesh.GetLabelList()[i];
		if(tSubsetLabel->GetLabelName() == FMM_MAKE_LABELNAME8(fcyModelSubsetLabel::LABELNAME))
		{
			fcyModelSubsetLabel* pLabel = (fcyModelSubsetLabel*)*tSubsetLabel;

			(*pOut)->SetSubset(tID, 
				(F2DPRIMITIVETYPE)pLabel->GetPrimitiveType(),
				pLabel->GetStartIndex(),
				pLabel->GetPrimitiveCount()
				);
			
			tInfo.m_IdToSubsetIndex[tID] = tInfo.m_SubsetList.size();
			tInfo.m_SubsetList.push_back(pLabel);
			tInfo.m_SubsetNameToId[pLabel->GetSubsetName()] = tID;

			tID++;
		}
	}

	// 遍历拷贝材质
	for(fuInt i = 0; i<Mesh.GetLabelList().size(); ++i)
	{
		fcyRefPointer<fcyModelLabel> tMaterialLabel = Mesh.GetLabelList()[i];
		if(tMaterialLabel->GetLabelName() == FMM_MAKE_LABELNAME8(fcyModelMaterialLabel::LABELNAME))
		{
			fcyModelMaterialLabel* pLabel = (fcyModelMaterialLabel*)*tMaterialLabel;

			tInfo.m_MaterialList[pLabel->GetMaterialName()] = pLabel;
		}
	}

	return tInfo;
}
