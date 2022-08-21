#include "fcyParser/fcyXml.h"

#include "fcyMisc/fcyStringHelper.h"
#include "fcyIO/fcyBinaryHelper.h"
#include <Windows.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyXmlException::fcyXmlException(fcStr Src, fcyXmlDocument* pOwner, fcStr Desc, ...)
	: fcyException(Src, "fancyXmlException : ")
{
	char szText[512];
	va_list marker;
	va_start(marker, Desc);
	vsprintf_s(szText, Desc, marker);
	va_end(marker);

	m_ExcpDesc += szText;
}

fcyXmlAttributeNotFount::fcyXmlAttributeNotFount(fcStr Src, fcyXmlDocument* pOwner, fcStrW Name)
	: fcyXmlException(Src, pOwner, "Element attribute '%s' not found.", fcyStringHelper::WideCharToMultiByte(Name).c_str())
{}

fcyXmlNodeNotFount::fcyXmlNodeNotFount(fcStr Src, fcyXmlDocument* pOwner, fcStrW Name)
	: fcyXmlException(Src, pOwner, "Element node '%s' not found.", fcyStringHelper::WideCharToMultiByte(Name).c_str())
{}

fcyXmlIndexOutOfRange::fcyXmlIndexOutOfRange(fcStr Src, fcyXmlDocument* pOwner, uint32_t Index)
	: fcyXmlException(Src, pOwner, "Index %d out of range.", Index)
{}

fcyXmlNodeHasDifferentOwner::fcyXmlNodeHasDifferentOwner(fcStr Src, fcyXmlDocument* pOwner, fcStrW NodeA, fcStrW NodeB)
	: fcyXmlException(Src, pOwner, "Owner between '%s' and '%s' is different.", 
		fcyStringHelper::WideCharToMultiByte(NodeA).c_str(),
		fcyStringHelper::WideCharToMultiByte(NodeA).c_str())
{}

fcyXmlNodeHasParent::fcyXmlNodeHasParent(fcStr Src, fcyXmlDocument* pOwner, fcStrW Name)
	: fcyXmlException(Src, pOwner, "Node '%s' already has parent.", fcyStringHelper::WideCharToMultiByte(Name).c_str())
{}

fcyXmlNodeIsInUse::fcyXmlNodeIsInUse(fcStr Src, fcyXmlDocument* pOwner, fcStrW Name)
	: fcyXmlException(Src, pOwner, "Node '%s' is in use.", fcyStringHelper::WideCharToMultiByte(Name).c_str())
{}

////////////////////////////////////////////////////////////////////////////////

fcyXmlElementList::fcyXmlElementList()
{}

fcyXmlElementList::fcyXmlElementList(const fcyXmlElementList& Org)
	: m_List(Org.m_List) 
{}

fcyXmlElementList::fcyXmlElementList(fcyXmlElementList&& Org)
	: m_List(std::move(Org.m_List))
{}

fcyXmlElement* fcyXmlElementList::operator[](uint32_t Index)
{
	if(Index >= m_List.size())
		throw fcyXmlIndexOutOfRange("fcyXmlElementList::operator[]", NULL, Index);

	return m_List[Index];
}

fcyXmlElementList& fcyXmlElementList::operator=(const fcyXmlElementList& Right)
{
	m_List = Right.m_List;
	return *this;
}

void fcyXmlElementList::Append(fcyXmlElement* pObj)
{
	m_List.push_back(pObj);
}

void fcyXmlElementList::Remove(uint32_t Index)
{
	if(Index >= m_List.size())
		throw fcyXmlIndexOutOfRange("fcyXmlElementList::operator[]", NULL, Index);

	vector<fcyXmlElement*>::iterator i = m_List.begin() + Index;
	m_List.erase(i);
}

void fcyXmlElementList::Clear()
{
	m_List.clear();
}

////////////////////////////////////////////////////////////////////////////////

fcyXmlElement::fcyXmlElement(fcyXmlDocument* pParent, const std::wstring& Name)
	: m_Name(Name), m_pParent(pParent), m_pParentNode(NULL)
{}

fcyXmlElement::~fcyXmlElement()
{}

fcyXmlElement* fcyXmlElement::GetNode(uint32_t Index)const
{
	if(Index >= m_Subnodes.size())
		throw fcyXmlIndexOutOfRange("fcyXmlElement::GetNode", m_pParent, Index);

	return m_Subnodes[Index];
}

fcyXmlElement* fcyXmlElement::GetFirstNode(const std::wstring& Name)const
{
	SubnodeConstIter i = m_Subnodes.begin();
	while(i != m_Subnodes.end())
	{
		if((*i)->GetName() == Name)
			return (*i);
		else
			++i;
	}

	return NULL;
}

fcyXmlElementList fcyXmlElement::GetNodeByName(const std::wstring& Name)const
{
	fcyXmlElementList tRet;

	SubnodeConstIter i = m_Subnodes.begin();
	while(i != m_Subnodes.end())
	{
		if((*i)->GetName() == Name)
			tRet.Append(*i);
		
		++i;
	}

	return std::move(tRet);
}

void fcyXmlElement::AppendNode(fcyXmlElement* pNode)
{
	if(pNode->GetDocument() != m_pParent)
		throw fcyXmlNodeHasDifferentOwner("fcyXmlElement::AppendNode", m_pParent, this->GetName().c_str(), pNode->GetName().c_str());
	if(pNode->GetParent() != NULL || m_pParent->GetRootElement() == pNode)
		throw fcyXmlNodeHasParent("fcyXmlElement::AppendNode", m_pParent, pNode->GetName().c_str());
	
	m_Subnodes.push_back(pNode);
	pNode->m_pParentNode = this;
}

void fcyXmlElement::RemoveNode(fcyXmlElement* pNode)
{
	SubnodeIter i = m_Subnodes.begin();
	while(i != m_Subnodes.end())
	{
		if((*i) == pNode)
		{
			(*i)->m_pParent = NULL;
			m_Subnodes.erase(i);
			return;
		}
		
		++i;
	}

	throw fcyXmlNodeNotFount("fcyXmlElement::RemoveNode", m_pParent, pNode->GetName().c_str());
}

void fcyXmlElement::RemoveNodeAt(uint32_t Index)
{
	if(Index >= m_Subnodes.size())
		throw fcyXmlIndexOutOfRange("fcyXmlElement::RemoveNodeAt", m_pParent, Index);

	SubnodeIter i = m_Subnodes.begin() + Index;
	(*i)->m_pParent = NULL;
	m_Subnodes.erase(i);
}

void fcyXmlElement::ClearNodes()
{
	SubnodeIter i = m_Subnodes.begin();
	while(i != m_Subnodes.end())
	{
		(*i)->m_pParent = NULL;
		m_Subnodes.erase(i);
		
		++i;
	}
}

const std::wstring& fcyXmlElement::GetAttribute(const std::wstring& Name)const
{
	AttrConstIter i = m_Attribute.find(Name);
	if(i == m_Attribute.end())
		throw fcyXmlAttributeNotFount("fcyXmlElement::GetAttribute", m_pParent, Name.c_str());

	return i->second;
}

void fcyXmlElement::SetAttribute(const std::wstring& Name, const std::wstring& Value)
{
	m_Attribute[Name] = Value;
}

void fcyXmlElement::SetAttribute(std::wstring&& Name, std::wstring&& Value)
{
	m_Attribute.insert(move(pair<wstring, wstring>(Name, Value)));
}

bool fcyXmlElement::HasAttribute(const std::wstring& Name)const
{
	AttrConstIter i = m_Attribute.find(Name);
	return (i != m_Attribute.end());
}

fcyXmlAttributeIterator fcyXmlElement::GetAttributeIter(const std::wstring& Name)
{
	return m_Attribute.find(Name);
}

const fcyXmlAttributeConstIterator fcyXmlElement::GetAttributeIter(const std::wstring& Name)const
{
	return m_Attribute.find(Name);
}

fcyXmlAttributeIterator fcyXmlElement::GetFirstAttributeIter()
{
	return m_Attribute.begin();
}

const fcyXmlAttributeConstIterator fcyXmlElement::GetFirstAttributeIter()const
{
	return m_Attribute.begin();
}

fcyXmlAttributeIterator fcyXmlElement::GetLastAttributeIter()
{
	return m_Attribute.end();
}

const fcyXmlAttributeConstIterator fcyXmlElement::GetLastAttributeIter()const
{
	return m_Attribute.end();
}

void fcyXmlElement::RemoveAttribute(const std::wstring& Name)
{
	AttrIter i = m_Attribute.find(Name);
	if(i == m_Attribute.end())
		throw fcyXmlAttributeNotFount("fcyXmlElement::RemoveAttribute", m_pParent, Name.c_str());

	m_Attribute.erase(i);
}

fcyXmlAttributeIterator fcyXmlElement::RemoveAttribute(const fcyXmlAttributeIterator& Iter)
{
	return m_Attribute.erase(Iter.i);
}

void fcyXmlElement::Save(std::wstring& pOut, uint32_t Indentation)const
{
	// 缩进
	for(uint32_t i = 0; i<Indentation; ++i)
	{
		pOut += L"\t";
	}

	// 写出标签头
	pOut += L"<" + m_Name;
	
	// 写属性
	if(m_Attribute.size())
	{
		pOut += L' ';

		map<wstring, wstring>::const_iterator i = m_Attribute.begin();
		while(i != m_Attribute.end())
		{
			pOut += i->first + L"=\"";
			
			// 写出属性值并转义
			for(uint32_t k = 0; k<i->second.size(); k++)
			{
				switch(i->second[k])
				{
				case L'<':
					pOut += L"&lt;";
					break;
				case L'>':
					pOut += L"&gt;";
					break;
				case L'&':
					pOut += L"&amp;";
					break;
				case L'\'':
					pOut += L"&apos;";
					break;
				case L'"':
					pOut += L"&quot;";
					break;
				default:
					pOut += i->second[k];
					break;
				}
			}

			pOut += L"\" ";
			++i;
		}
	}

	// 结尾？
	if(m_Content.empty() && m_Subnodes.size()==0)
	{
		pOut += L"/>\n";
		return;
	}
	else
	{
		pOut += L">";
	}

	// 写出子节点
	if(m_Subnodes.size())
	{
		// 换行
		pOut += L"\n";

		vector<fcyXmlElement*>::const_iterator i = m_Subnodes.begin();

		while(i != m_Subnodes.end())
		{
			(*i)->Save(pOut, Indentation + 1);
			
			++i;
		}

		// 缩进
		for(uint32_t i = 0; i<Indentation; ++i)
		{
			pOut += L"\t";
		}

		// 闭合标签
		pOut += L"</" + m_Name + L">\n";
	}
	else if(m_Content.size())
	{
		// 写出Content
		for(uint32_t i = 0; i<m_Content.size(); ++i)
		{
			switch(m_Content[i])
			{
			case L'<':
				pOut += L"&lt;";
				break;
			case L'>':
				pOut += L"&gt;";
				break;
			case L'&':
				pOut += L"&amp;";
				break;
			case L'\'':
				pOut += L"&apos;";
				break;
			case L'"':
				pOut += L"&quot;";
				break;
			default:
				pOut += m_Content[i];
				break;
			}
		}

		// 闭合标签
		pOut += L"</" + m_Name + L">\n";
	}
}

fcyXmlElement* fcyXmlElement::Clone(fcyXmlDocument* pDoc)const
{
	fcyXmlElement* pThis = pDoc->CreateElement(GetName());
	pThis->SetContent(GetContent());
	pThis->m_Attribute = m_Attribute;

	// 克隆子节点
	for(SubnodeConstIter i = m_Subnodes.begin(); i != m_Subnodes.end(); ++i)
	{
		fcyXmlElement* tSub = (*i)->Clone(pDoc);

		pThis->AppendNode(tSub);
	}

	return pThis;
}

////////////////////////////////////////////////////////////////////////////////

bool fcyXmlDocument::checkUTF8(fcyStream* pStream)
{
	uint64_t tPos = pStream->GetPosition();

	// 尝试读取BOM头
	uint8_t tBomCache[3];
	if(FCYOK(pStream->ReadBytes(tBomCache, 3, NULL)))
	{
		uint8_t tUTF8[3] = { 0xEF, 0xBB, 0xBF };
		if(memcmp(tBomCache, tUTF8, 3)==0)
			return true;
		else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);
	}
	else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);

	return false;
}

bool fcyXmlDocument::checkUTF16LE(fcyStream* pStream)
{
	uint64_t tPos = pStream->GetPosition();

	// 尝试读取BOM头
	uint8_t tBomCache[2];
	if(FCYOK(pStream->ReadBytes(tBomCache, 2, NULL)))
	{
		uint8_t tUTF16LE[2] = { 0xFF, 0xFE };
		if(memcmp(tBomCache, tUTF16LE, 2)==0)
			return true;
		else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);
	}
	else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);

	return false;
}

wstring fcyXmlDocument::preprocessXml(fcyStream* pStream)
{
	// 在有BOM头的时候忽略预处理指令
	if(checkUTF16LE(pStream))
	{
		wstring tRet;
		uint64_t tSize = pStream->GetLength() - pStream->GetPosition();
		tRet.resize((size_t)tSize);
		pStream->ReadBytes((fData)&tRet[0], tSize, NULL);
		return tRet;
	}
	else if(checkUTF8(pStream))
	{
		string tTemp;
		uint64_t tSize = pStream->GetLength() - pStream->GetPosition();
		tTemp.resize((size_t)tSize);
		pStream->ReadBytes((fData)&tTemp[0], tSize, NULL);
		return fcyStringHelper::MultiByteToWideChar(tTemp, CP_UTF8);
	}
	else
	{
		// 依次读取前面的字符，解析预处理指令
		int32_t tCodePage = CP_UTF8;
		uint64_t tPos = pStream->GetPosition();

		fcyBinaryReader tReader(pStream);

		try
		{
			char tFlag[6] = { 0 };
			tReader.ReadChars(tFlag, 5);
			if(strcmp(tFlag, "<?xml")==0) // 匹配<?xml
			{
				map<string, string> tAttrib;
				string tKey;
				string tValue;

				while(1)
				{
					char tChar = tReader.ReadChar();

					if(isspace(tChar))
						continue;
					else if(tChar == '?')
					{
						tChar = tReader.ReadChar();
						if(tChar == '>')
							break;
						else
							throw fcyException("fcyXmlDocument::preprocessXml", "expect >");
					}
					else if(tChar == '=')
					{
						if(tKey.empty())
							throw fcyException("fcyXmlDocument::preprocessXml", "expect name but found =");
						
						// 读取value
						bool tMatch = false;
						while(1)
						{
							char tChar = tReader.ReadChar();
							
							if(isspace(tChar))
								continue;
							else if(tChar == '"')
							{
								if(tMatch == true)
									break;
								else
									tMatch = true;
							}
							else
							{
								tValue += tChar;
							}
						}
						tAttrib[tKey] = tValue;
						tKey = tValue = "";
					}
					else
					{
						tKey += tChar;
					}
				}

				// 获取信息
				if(tAttrib.find("version")!=tAttrib.end() && atof(tAttrib["version"].c_str()) != 1.0)
					throw fcyException("fcyXml::preprocessXml", "xml version not support");
				if(tAttrib.find("encoding")!=tAttrib.end())
				{
					if(tAttrib["encoding"] == "UTF-8")
						tCodePage = CP_UTF8;
					else if(tAttrib["encoding"] == "gb2312")
						tCodePage = 936;
					else
						throw fcyException("fcyXmlDocument::preprocessXml", "unknown encoding");
				}
			}
			else
				throw fcyException("fcyXmlDocument::preprocessXml", "expect <?xml");
		}
		catch(...)
		{
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);
		}

		// 读取并转换
		string tTemp;
		uint64_t tSize = pStream->GetLength() - pStream->GetPosition();
		tTemp.resize((size_t)tSize);
		pStream->ReadBytes((fData)&tTemp[0], tSize, NULL);
		return fcyStringHelper::MultiByteToWideChar(tTemp, tCodePage);
	}
}

bool fcyXmlDocument::ignoreComment(fcyLexicalReader& tReader)
{
	if(tReader.TryMatch(L"<!--", true, true))
	{
		while(1)
		{
			wchar_t tChar = tReader.ReadChar();

			if(tChar == L'-')
			{
				if(tReader.TryMatch(L"->", false, true))
					return true;
			}
		}
	}
	return false;
}

bool fcyXmlDocument::ignorePreprocess(fcyLexicalReader& tReader)
{
	if(tReader.TryMatch(L"<?xml", true, true))
	{
		map<wstring, wstring> tAttrib;
		wstring tKey;
		wstring tValue;

		while(1)
		{
			wchar_t tChar = tReader.ReadChar();

			if(isspace(tChar))
				continue;
			else if(tChar == L'?')
			{
				tChar = tReader.ReadChar();
				if(tChar == L'>')
					break;
				else
					throw fcyException("fcyXmlDocument::ignorePreprocess", "expect >");
			}
			else if(tChar == L'=')
			{
				if(tKey.empty())
					throw fcyException("fcyXmlDocument::ignorePreprocess", "expect name but found =");

				// 读取value
				bool tMatch = false;
				while(1)
				{
					wchar_t tChar = tReader.ReadChar();

					if(isspace(tChar))
						continue;
					else if(tChar == L'"')
					{
						if(tMatch == true)
							break;
						else
							tMatch = true;
					}
					else
					{
						tValue += tChar;
					}
				}
				tAttrib[tKey] = tValue;
				tKey = tValue = L"";
			}
			else
			{
				tKey += tChar;
			}
		}
		return true;
	}
	return false;
}

bool fcyXmlDocument::tryReadCDATA(fcyLexicalReader& tReader, std::wstring& tOut)
{
	tOut = L"";
	if(tReader.TryMatch(L"<![CDATA[", false, true))
	{
		while(!tReader.TryMatch(L"]]>", false, true))
		{
			tOut.push_back(tReader.ReadChar());
		}

		return true;
	}
	return false;
}

wchar_t fcyXmlDocument::praseEscape(fcyLexicalReader& tReader)
{
	if(tReader.TryMatch(L"&#", false, true))
	{
		wstring tNum;
		wchar_t tChar;
		while((tChar = tReader.ReadChar())!=L';')
		{
			tNum += tChar;
		}
		return (wchar_t)_wtoi(tNum.c_str());
	}
	if(tReader.TryMatch(L"&lt;", false, true))
		return L'<';
	if(tReader.TryMatch(L"&gt;", false, true))
		return L'>';
	if(tReader.TryMatch(L"&amp;", false, true))
		return L'&';
	if(tReader.TryMatch(L"&apos;", false, true))
		return L'\'';
	if(tReader.TryMatch(L"&quot;", false, true))
		return L'\"';
	throw fcyLexicalException("fcyXmlDocument::praseEscape", "Invalid escape char.", tReader.GetLine(), tReader.GetRow());
}

std::wstring fcyXmlDocument::readName(fcyLexicalReader& tReader)
{
	wstring tRet;
	       
	wchar_t tChar = tReader.PeekChar();
	if(tChar == L'/')  // 不能以'/'开头
		throw fcyLexicalException("fcyXmlDocument::readName", "Name can't begin with '/'.", tReader.GetLine(), tReader.GetRow());
	if(tChar == L'_')  // 不能以下划线开头
		throw fcyLexicalException("fcyXmlDocument::readName", "Name can't begin with '_'.", tReader.GetLine(), tReader.GetRow());
	if(iswspace(tChar)) // 不能以空白符开头
		throw fcyLexicalException("fcyXmlDocument::readName", "Name can't begin with space.", tReader.GetLine(), tReader.GetRow());
	if(iswdigit(tChar)) // 不能以数字开头
		throw fcyLexicalException("fcyXmlDocument::readName", "Name can't begin with digit.", tReader.GetLine(), tReader.GetRow());
	
	while(!(iswspace(tChar) || tChar==L'/' || tChar==L'<' || tChar==L'>' || tChar==L'"' || tChar==L'=' || tChar==L'&'))
	{
		tRet += tChar;
		tReader.ReadChar();
		tChar = tReader.PeekChar();
	}

	return std::move(tRet);
}

std::wstring fcyXmlDocument::readString(fcyLexicalReader& tReader)
{
	wstring tRet;

	// 读取'"'
	tReader.Match(L'"', true);

	while(1)
	{
		wchar_t tChar = tReader.PeekChar();

		if(tChar == L'"')
		{
			tReader.ReadChar();
			break;
		}
		else if(tChar == L'&') // 处理escape
		{
			tRet += praseEscape(tReader);
			continue;
		}

		tRet += tChar;
		tChar = tReader.ReadChar();
	}

	return std::move(tRet);
}

void fcyXmlDocument::readAttribute(fcyLexicalReader& tReader, fcyXmlElement* pNode)
{
	while(!(tReader.TryMatch(L'>', true, false) || tReader.TryMatch(L"/>", true, false)))
	{
		// 读取属性名
		wstring tName = readName(tReader);

		// 读取'='
		tReader.Match(L'=', true);

		// 读取字符串
		wstring tStr = readString(tReader);

		tReader.IgnoreSpace();

		// 加入属性
		pNode->SetAttribute(std::move(tName), std::move(tStr));
	}
}

void fcyXmlDocument::readNodes(fcyLexicalReader& tReader, fcyXmlElement* pNode)
{
	wstring tContent;

	// 去注释
	while(ignoreComment(tReader));

	// 检查分支
	if(tReader.TryMatch(L'<', true, false) && !tReader.TryMatch(L"<![CDATA[", true, false))
	{
		// 作为子节点
		while(!tReader.TryMatch(L"</", true, false))
		{
			// 解析并加入标签
			pNode->AppendNode(parserNode(tReader));

			// 去注释
			while(ignoreComment(tReader));
		}
	}
	else
	{
		// 作为文本节点
		wchar_t tChar = tReader.PeekChar();
		while(true)
		{
			bool tEnd = false;

			// 处理CDATA和注释
			while(tChar == L'<')
			{
				ignoreComment(tReader);

				tChar = tReader.PeekChar();
				
				if(tChar == L'<')
				{
					wstring tData;

					if(tryReadCDATA(tReader, tData))
					{
						tContent += tData;
						tChar = tReader.PeekChar();
					}
					else
					{
						tEnd = true;
						break;
					}
				}
			}

			if(tEnd)
				break;

			// 处理escape
			if(tChar == L'&')
			{
				tContent += praseEscape(tReader);
				tChar = tReader.PeekChar();
				continue;
			}

			tContent += tChar;
			tReader.ReadChar();
			tChar = tReader.PeekChar();
		}

		pNode->SetContent(tContent);
	}
}

fcyXmlElement* fcyXmlDocument::parserNode(fcyLexicalReader& tReader)
{
	// 读取'<'
	tReader.Match(L'<', true);

	// 读取名称
	std::wstring tNodeName = readName(tReader);

	// 创建节点
	fcyXmlElement* pElement = CreateElement(tNodeName);

	tReader.IgnoreSpace();

	// 读取属性
	readAttribute(tReader, pElement);

	wchar_t tChar = tReader.ReadChar();
	if(tChar == L'/')
	{
		// 分支 <1> 遇到'/',完成节点解析

		// 读取'>'
		tReader.Match(L'>', false);
	}
	else if(tChar == L'>')
	{
		// 分支 <2> 解析子节点

		// 读取子节点或文本
		readNodes(tReader, pElement);

		// 读取'</'
		tReader.Match(L"</", true);

		// 读取名称
		wstring tName = readName(tReader);

		if(tName != tNodeName)
		{
			wchar_t tText[512];
			swprintf_s(tText, L"Name <%s> not match <%s>.", tName.c_str(), tNodeName.c_str());
			throw fcyLexicalException("fcyXmlDocument::parserNode", 
				fcyStringHelper::WideCharToMultiByte(tText).c_str(),
				tReader.GetLine(), tReader.GetRow());
		}

		// 读取'>'
		tReader.Match(L'>', true);
	}
	else
	{
		wchar_t tText[512];
		swprintf_s(tText, L"Unexpected character '%c'.", tChar);
		throw fcyLexicalException("fcyXmlDocument::parserNode", 
			fcyStringHelper::WideCharToMultiByte(tText).c_str(),
			tReader.GetLine(), tReader.GetRow());
	}

	return pElement;
}

fcyXmlDocument::fcyXmlDocument()
	: m_pRootElement(NULL)
{}

fcyXmlDocument::fcyXmlDocument(const std::wstring& Str)
	: m_pRootElement(NULL)
{
	fcyLexicalReader tReader(Str);
	
	// 去预处理
	while(ignorePreprocess(tReader));
	// 去注释
	while(ignoreComment(tReader));

	SetRootElement(parserNode(tReader));
}

fcyXmlDocument::fcyXmlDocument(fcyStream* pStream)
	: m_pRootElement(NULL)
{
	if(!pStream)
		throw fcyException("fcyXmlDocument::fcyXmlDocument", "Param 'pStream' is null.");

	fcyLexicalReader tReader(preprocessXml(pStream));

	// 去预处理
	while(ignorePreprocess(tReader));
	// 去注释
	while(ignoreComment(tReader));

	SetRootElement(parserNode(tReader));
}

fcyXmlDocument::fcyXmlDocument(fcyXmlDocument&& Org)
	: m_pRootElement(Org.m_pRootElement), m_ElementPool(std::move(Org.m_ElementPool))
{}

fcyXmlDocument::~fcyXmlDocument()
{
	vector<fcyXmlElement*>::iterator i = m_ElementPool.begin();
	while(i != m_ElementPool.end())
	{
		(*i)->~fcyXmlElement();
		m_ElementMem.Free(*i);

		++i;
	}
	m_ElementPool.clear();
}

void fcyXmlDocument::SetRootElement(fcyXmlElement* pRoot)
{
	if(!pRoot || m_pRootElement == pRoot)
		return;
	if(pRoot->GetDocument() != this)
		throw fcyXmlNodeNotFount("fcyXmlDocument::SetRootElement", this, pRoot->GetName().c_str());
	if(pRoot->GetParent() != NULL)
		throw fcyXmlNodeHasParent("fcyXmlDocument::SetRootElement", this, pRoot->GetName().c_str());
	
	m_pRootElement = pRoot;
}

fcyXmlElement* fcyXmlDocument::CreateElement(const std::wstring& Name)
{
	fcyXmlElement* pElement = new(m_ElementMem.Alloc()) fcyXmlElement(this, Name);

	m_ElementPool.push_back(pElement);

	return pElement;
}

void fcyXmlDocument::DeleteElement(fcyXmlElement* pObj)
{
	if(!pObj)
		return;
	if(pObj->GetDocument() != this)
		throw fcyXmlNodeNotFount("fcyXmlDocument::DeleteElement", this, pObj->GetName().c_str());
	if(pObj->GetParent() != NULL)
		throw fcyXmlNodeIsInUse("fcyXmlDocument::DeleteElement", this, pObj->GetName().c_str());

	vector<fcyXmlElement*>::iterator i = m_ElementPool.begin();
	while(i != m_ElementPool.end())
	{
		if((*i) == pObj)
			break;

		++i;
	}
	if(i == m_ElementPool.end())
		throw fcyXmlNodeNotFount("fcyXmlDocument::DeleteElement", this, pObj->GetName().c_str());

	m_ElementPool.erase(i);
	pObj->~fcyXmlElement();
	m_ElementMem.Free(pObj);
}

void fcyXmlDocument::Save(std::wstring& Out)const
{
	if(m_pRootElement)
		m_pRootElement->Save(Out, 0);
	else
		throw fcyException("fcyXmlDocument::Save", "Root element is null.");
}

void fcyXmlDocument::Save(fcyStream* pOut)const
{
	wstring tOutStr;

	if(m_pRootElement)
		m_pRootElement->Save(tOutStr, 0);
	else
		throw fcyException("fcyXmlDocument::Save", "Root element is null.");

	// UTF 16 BOM
	uint8_t tUTF16LE[2] = { 0xFF, 0xFE };
	pOut->SetLength(0);
	pOut->WriteBytes(tUTF16LE, 2, NULL);
	pOut->WriteBytes((fData)&tOutStr[0], tOutStr.length() * sizeof(wchar_t), NULL);
}
