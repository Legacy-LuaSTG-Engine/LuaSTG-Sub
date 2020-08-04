#include "fcyFile/fcyResPackage.h"

#include "fcyIO/fcyBinaryHelper.h"
#include "fcyIO/fcyDeflate.h"
#include "fcyIO/fcyStream.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyResPackageNode::fcyResPackageNode(fcyResPackageFloderNode* pParent, const wstring& Name)
	: m_pParent(pParent), m_Name(Name)
{}

fcyResPackageNode::~fcyResPackageNode()
{}

fcyResPackageFloderNode* fcyResPackageNode::GetParent()
{
	return m_pParent;
}

fcStrW fcyResPackageNode::GetName()
{
	return m_Name.c_str();
}

fResult fcyResPackageNode::SetName(fcStrW Name)
{
	if(m_pParent->Contains(Name))
		return FCYERR_OBJEXSITED;
	m_Name = Name;
	return FCYERR_OK;
}

////////////////////////////////////////////////////////////////////////////////

fcyResPackageDataNode::fcyResPackageDataNode(fcyResPackageFloderNode* pParent, const wstring& Name)
	: fcyResPackageNode(pParent, Name),
	m_pStream(NULL), m_DataOffset(0), m_DataRealSize(0), m_DataCompressedSize(0), m_bDataCompressed(false)
{}

fcyResPackageDataNode::~fcyResPackageDataNode()
{
	FCYSAFEKILL(m_pStream);
}

fuInt fcyResPackageDataNode::GetDataOffset()
{
	return m_DataOffset;
}

fuInt fcyResPackageDataNode::GetDataRealSize()
{
	return m_DataRealSize;
}

fuInt fcyResPackageDataNode::GetDataCompressedSize()
{
	return m_DataCompressedSize;
}

fBool fcyResPackageDataNode::IsDataCompressed()
{
	return m_bDataCompressed;
}

fResult fcyResPackageDataNode::DumpStream(fcyStream** pOut)
{
	if(!pOut)
		return FCYERR_INVAILDPARAM;

	if(GetDataRealSize() == 0)
		return FCYERR_OBJNOTEXSIT;

	*pOut = NULL;

	fcyPartialStream* pPartialStream = new fcyPartialStream(
		m_pStream, 
		m_DataOffset + 12, 
		m_bDataCompressed ? m_DataCompressedSize : m_DataRealSize);

	if(m_bDataCompressed)
	{
		fcyStream* pOutStream = NULL;
		if(FCYFAILED(fcyDeflate::DecompressStreamToMem(pPartialStream, &pOutStream)))
		{
			FCYSAFEKILL(pPartialStream);
			return FCYERR_INVAILDDATA;
		}
		else
		{
			FCYSAFEKILL(pPartialStream);
			*pOut = pOutStream;
			pOutStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
			return FCYERR_OK;
		}
	}
	else
	{
		*pOut = pPartialStream;
		pPartialStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
		return FCYERR_OK;
	}
}

fcStrW fcyResPackageDataNode::GetAdditonStr()
{
	return m_AdditionStr.c_str();
}

void fcyResPackageDataNode::SetAdditionStr(fcStrW Src)
{
	m_AdditionStr = Src;
}

fResult fcyResPackageDataNode::Read(fcyStream* pStream, fcyXmlElement* pXmlNode)
{
	if(pXmlNode->GetName() != L"Node")
		return FCYERR_INVAILDDATA;

	FCYSAFEKILL(m_pStream);
	m_pStream = pStream;
	m_pStream->AddRef();

	if(pXmlNode->HasAttribute(L"OrgSize"))
		m_DataRealSize = _wtoi(pXmlNode->GetAttribute(L"OrgSize").c_str());
	else
		return FCYERR_INVAILDDATA;

	if(pXmlNode->HasAttribute(L"CompressedSize"))
		m_DataCompressedSize = _wtoi(pXmlNode->GetAttribute(L"CompressedSize").c_str());
	else
		return FCYERR_INVAILDDATA;

	if(pXmlNode->HasAttribute(L"Offset"))
		m_DataOffset = _wtoi(pXmlNode->GetAttribute(L"Offset").c_str());
	else
		return FCYERR_INVAILDDATA;

	if(pXmlNode->HasAttribute(L"IsCompressed"))
		m_bDataCompressed = pXmlNode->GetAttribute(L"IsCompressed") == L"yes" ? true : false;
	else
		return FCYERR_INVAILDDATA;

	m_AdditionStr = pXmlNode->GetContent();

	return FCYERR_OK;
}

fcyResPackageDataNode* fcyResPackageDataNode::ToDataNode()
{
	return this;
}

fcyResPackageFloderNode* fcyResPackageDataNode::ToFloderNode()
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////

fcyResPackageFloderNode::fcyResPackageFloderNode(fcyResPackageFloderNode* pParent, const wstring& Name)
	: fcyResPackageNode(pParent, Name)
{}

fcyResPackageFloderNode::~fcyResPackageFloderNode()
{
	Clear();
}

fuInt fcyResPackageFloderNode::GetNodeCount()
{
	return m_NodeList.size();
}

fcyResPackageNode* fcyResPackageFloderNode::GetNode(fuInt Index)
{
	if(Index >= GetNodeCount())
		return NULL;

	return m_NodeList[Index];
}

fcyResPackageNode* fcyResPackageFloderNode::GetNode(fcStrW Name)
{
	vector<fcyResPackageNode*>::iterator i = m_NodeList.begin();
	
	while( i != m_NodeList.end() )
	{
		if(wcscmp((*i)->GetName(), Name)==0)
			return (*i);
		++i;
	}

	return NULL;
}

fBool fcyResPackageFloderNode::Contains(fcStrW Name)
{
	return GetNode(Name) != NULL;
}

fBool fcyResPackageFloderNode::Append(fcyResPackageNode* pNode)
{
	if(Contains(pNode->GetName()))
		return false;

	m_NodeList.push_back(pNode);

	return true;
}

fBool fcyResPackageFloderNode::Remove(fcStrW Name)
{
	vector<fcyResPackageNode*>::iterator i = m_NodeList.begin();
	
	while( i != m_NodeList.end() )
	{
		if(wcscmp((*i)->GetName(), Name)==0)
		{
			FCYSAFEDEL(*i);
			m_NodeList.erase(i);
			return true;
		}
		++i;
	}

	return false;
}

void fcyResPackageFloderNode::Clear()
{
	vector<fcyResPackageNode*>::iterator i = m_NodeList.begin();
	
	while( i != m_NodeList.end() )
	{
		FCYSAFEDEL(*i);
		++i;
	}

	m_NodeList.clear();
}

fResult fcyResPackageFloderNode::Read(fcyStream* pStream, fcyXmlElement* pXmlNode)
{
	Clear();

	fuInt tCount = pXmlNode->GetNodeCount();
	for(fuInt i = 0; i<tCount; ++i)
	{
		fcyXmlElement* pSubXmlNode = pXmlNode->GetNode(i);
		fcyResPackageNode* tNode = NULL;
		const wstring& tNamePtr = pSubXmlNode->GetAttribute(L"Name");

		if(pSubXmlNode->GetName() == L"Floder")
		{
			tNode = new fcyResPackageFloderNode(this, tNamePtr);
			if(FCYFAILED(tNode->Read(pStream, pSubXmlNode)))
			{
				FCYSAFEDEL(tNode);
				return FCYERR_INTERNALERR;
			}
		}
		else
		{
			tNode = new fcyResPackageDataNode(this, tNamePtr);
			if(FCYFAILED(tNode->Read(pStream, pSubXmlNode)))
			{
				FCYSAFEDEL(tNode);
				return FCYERR_INTERNALERR;
			}
		}

		m_NodeList.push_back(tNode);
	}

	return FCYERR_OK;
}

fcyResPackageDataNode* fcyResPackageFloderNode::ToDataNode()
{
	return NULL;
}

fcyResPackageFloderNode* fcyResPackageFloderNode::ToFloderNode()
{
	return this;
}

////////////////////////////////////////////////////////////////////////////////

fcyResPackage::fcyResPackage(fcyStream* pStream)
	: m_Root(NULL, L"Res")
{
	if(FCYFAILED(loadFromStream(pStream)))
		throw fcyException("fcyResPackage::fcyResPackage", "loadFromStream Failed.");
}

fcyResPackage::~fcyResPackage(void)
{
}

fResult fcyResPackage::loadFromStream(fcyStream* pStream)
{
	fcyBinaryReader tReader(pStream);

	// 读取数据头
	m_Flag = tReader.ReadUInt32();
	m_Version = tReader.ReadUInt32();
	m_IndexOffset = tReader.ReadUInt32();

	if(m_Flag != 0x00707266 && m_Version != 1)
		return FCYERR_NOTSUPPORT;

	// 跳到主索引区
	pStream->SetPosition(FCYSEEKORIGIN_BEG, m_IndexOffset);

	// 读取Xml数据大小
	fuInt tXmlDataSize = tReader.ReadUInt32();

	// 解压Xml
	fcyStream* pMemStream = NULL; 
	if(FCYFAILED(fcyDeflate::DecompressStreamToMem(pStream, &pMemStream)))
		return FCYERR_INVAILDDATA;

	// 检查数据
	if(pMemStream->GetLength() != tXmlDataSize)
	{
		FCYSAFEKILL(pMemStream);
		return FCYERR_INVAILDDATA;
	}

	// 读取Xml
	wstring tXmlStr;
	tXmlStr.resize((wstring::size_type)pMemStream->GetLength());
	pMemStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
	pMemStream->ReadBytes((fData)&tXmlStr[0], tXmlStr.size(), NULL);
	FCYSAFEKILL(pMemStream);

	fcyXmlDocument tXml(tXmlStr);
	if(FCYFAILED(m_Root.Read(pStream, tXml.GetRootElement())))
		return FCYERR_INTERNALERR;

	return FCYERR_OK;
}

fcyResPackageFloderNode* fcyResPackage::GetRoot()
{
	return &m_Root;
}