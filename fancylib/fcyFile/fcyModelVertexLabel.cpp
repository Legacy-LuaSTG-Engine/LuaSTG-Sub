#include "fcyModelVertexLabel.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const char fcyModelVertexLabel::LABELNAME[8] = { "VERTEX" };

fuInt fcyModelVertexLabel::CalcuVertexSize()const
{
	fuInt tVertSize = 0;

	for(fuInt i = 0; i<m_VertElementList.size(); ++i)
	{
		switch(m_VertElementList[i].Type)
		{
		case ELEMENTTYPE_FLOAT:
			tVertSize += 4; 
			break;
		case ELEMENTTYPE_FLOAT2:
			tVertSize += 4 * 2; 
			break;
		case ELEMENTTYPE_FLOAT3:
			tVertSize += 4 * 3; 
			break;
		case ELEMENTTYPE_FLOAT4:
			tVertSize += 4 * 4; 
			break;
		case ELEMENTTYPE_COLOR:
		case ELEMENTTYPE_UBYTE4:
			tVertSize += 4; 
			break;
		case ELEMENTTYPE_SHORT2:
			tVertSize += 2 * 2; 
			break;
		case ELEMENTTYPE_SHORT4:
			tVertSize += 2 * 4; 
			break;
		default:
			throw fcyException("fcyModelVertexLabel::CalcuVertexSize", "Unrecognized element type.");
		}
	}

	return tVertSize;
}

void fcyModelVertexLabel::PushVertex(void* pData, fuInt VertexSize)
{
	// 分配空间
	m_VertexData.resize(m_VertexData.size() + VertexSize);

	// 复制内存
	memcpy(&m_VertexData[m_VertexData.size() - VertexSize], pData, VertexSize);
}

void fcyModelVertexLabel::ReadData(fcyStream* pStream)
{
	fcyBinaryReader tReader(pStream);

	// 清空数据
	m_VertElementList.clear();
	m_VertexData.clear();

	// 读取头部
	fuInt tVertexCount = tReader.ReadUInt32();
	fuInt tVertexElementCount = tReader.ReadUInt32();
	fByte tStreamFlag = tReader.ReadByte();

	// 目前只支持默认的流标志
	if(tStreamFlag != 0)
		throw fcyException("fcyModelVertexLabel::ReadData", "Unsupport Stream Type.");
	
	// 加载顶点声明
	m_VertElementList.resize(tVertexElementCount);
	for(fuInt i = 0; i<tVertexElementCount; ++i)
	{
		m_VertElementList[i].Type = (ELEMENTTYPE)tReader.ReadUInt32();
		m_VertElementList[i].Usage = tReader.ReadUInt64();
		m_VertElementList[i].UsageIndex = tReader.ReadByte();
	}

	// 计算顶点大小
	fuInt tVertSize = CalcuVertexSize();

	// 加载顶点数据
	m_VertexData.resize(tVertSize * tVertexCount);

	// 直接读取size数据并填充
	tReader.ReadBytes((fByte*)GetVertexDataPointer(), GetVertexDataSize());
}

void fcyModelVertexLabel::WriteData(fcyStream* pStream)
{
	fcyBinaryWriter tWritter(pStream);

	// 顶点数据检查
	fuInt tVertSize = CalcuVertexSize();
	if(tVertSize == 0 || m_VertexData.size() % tVertSize != 0)
		throw fcyException("fcyModelVertexLabel::WriteData", "Size of vertex is not correctly.");

	// 写出数据头
	tWritter.Write((fuInt)m_VertexData.size() / tVertSize);
	tWritter.Write((fuInt)m_VertElementList.size());
	tWritter.Write((fByte)0);  // 流标记

	// 写出顶点声明
	for(fuInt i = 0; i<m_VertElementList.size(); ++i)
	{
		tWritter.Write((fuInt)m_VertElementList[i].Type);
		tWritter.Write((fuLong)m_VertElementList[i].Usage);
		tWritter.Write((fByte)m_VertElementList[i].UsageIndex);
	}

	// 写出顶点数据
	tWritter.Write((fcData)GetVertexDataPointer(), GetVertexDataSize());
}
