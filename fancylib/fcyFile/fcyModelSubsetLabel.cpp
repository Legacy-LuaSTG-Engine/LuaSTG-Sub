#include "fcyModelSubsetLabel.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const char fcyModelSubsetLabel::LABELNAME[8] = { "SUBSET" };

void fcyModelSubsetLabel::ReadData(fcyStream* pStream)
{
	fcyBinaryReader tReader(pStream);

	m_SubsetName = fcyModelLabel::ReadString(pStream);
	m_MaterialName = fcyModelLabel::ReadString(pStream);
	m_PrimitiveType = (PRIMTYPE)tReader.ReadUInt32();
	m_StartIndex = tReader.ReadUInt32();
	m_PrimitiveCount = tReader.ReadUInt32();
}

void fcyModelSubsetLabel::WriteData(fcyStream* pStream)
{
	fcyBinaryWriter tWritter(pStream);

	fcyModelLabel::WriteString(pStream, m_SubsetName);
	fcyModelLabel::WriteString(pStream, m_MaterialName);
	tWritter.Write((fuInt)m_PrimitiveType);
	tWritter.Write((fuInt)m_StartIndex);
	tWritter.Write((fuInt)m_PrimitiveCount);
}
