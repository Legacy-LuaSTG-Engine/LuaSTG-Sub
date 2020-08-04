#include "fcyModelIndexLabel.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const char fcyModelIndexLabel::LABELNAME[8] = { "INDEX" };

fBool fcyModelIndexLabel::IsIndex32()
{
	fBool tRet = false;
	for(fuInt i = 0; i<m_IndexData.size(); ++i)
		if(m_IndexData[i] > 65535)
		{
			tRet = true;
			break;
		}
	
	return tRet;
}

void fcyModelIndexLabel::ReadData(fcyStream* pStream)
{
	fcyBinaryReader tReader(pStream);

	// 清空数据
	m_IndexData.clear();

	// 读取头部
	fuInt tIndexCount = tReader.ReadUInt32();
	fByte tUseInt32Index = tReader.ReadByte();
	fByte tStreamFlag = tReader.ReadByte();

	// 目前只支持默认的流数据
	if(tStreamFlag != 0)
		throw fcyException("fcyModelIndexLabel::ReadData", "Unsupport Stream Type.");
	
	// 读取索引数据
	m_IndexData.resize(tIndexCount);
	for(fuInt i = 0; i<tIndexCount; ++i)
	{
		if(tUseInt32Index)
			m_IndexData[i] = tReader.ReadUInt32();
		else
			m_IndexData[i] = tReader.ReadUInt16();
	}
}

void fcyModelIndexLabel::WriteData(fcyStream* pStream)
{
	fcyBinaryWriter tWritter(pStream);

	// 确定索引格式
	fBool tIndex32 = IsIndex32();

	// 写出头部
	tWritter.Write((fuInt)m_IndexData.size());
	tWritter.Write((fByte)tIndex32);
	tWritter.Write((fByte)0);                  // 流标记

	// 写出索引
	for(fuInt i = 0; i<m_IndexData.size(); ++i)
	{
		if(tIndex32)
			tWritter.Write((fuInt)m_IndexData[i]);
		else
			tWritter.Write((fuShort)m_IndexData[i]);
	}
}
