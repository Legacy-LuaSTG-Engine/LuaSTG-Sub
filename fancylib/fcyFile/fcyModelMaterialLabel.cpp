#include "fcyModelMaterialLabel.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const char fcyModelMaterialLabel::LABELNAME[8] = { 'M', 'A', 'T', 'E' , 'R', 'I', 'A', 'L' };

void fcyModelMaterialLabel::ReadData(fcyStream* pStream)
{
	fcyBinaryReader tReader(pStream);

	m_MatName = fcyModelLabel::ReadString(pStream);
	m_MatXMLData = fcyModelLabel::ReadString(pStream);
}

void fcyModelMaterialLabel::WriteData(fcyStream* pStream)
{
	fcyModelLabel::WriteString(pStream, m_MatName);
	fcyModelLabel::WriteString(pStream, m_MatXMLData);
}
