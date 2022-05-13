#include "Renderer/f2dFontTexProvider.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dFontTexProvider::f2dFontTexProvider(f2dStream* pDefineFile, f2dTexture2D* pTex)
	: m_OrgTex(pTex)
{
	if(!m_OrgTex)
		throw fcyException("f2dFontTexProvider::f2dFontTexProvider", "Param 'pTex' is null.");
	fcyXmlDocument tXml(pDefineFile);

	loadDefine(tXml);

	m_OrgTex->AddRef();
}

f2dFontTexProvider::f2dFontTexProvider(fcStrW pDefineFile, f2dTexture2D* pTex)
	: m_OrgTex(pTex)
{
	if(!m_OrgTex)
		throw fcyException("f2dFontTexProvider::f2dFontTexProvider", "Param 'pTex' is null.");
	fcyXmlDocument tXml(pDefineFile);

	loadDefine(tXml);

	m_OrgTex->AddRef();
}

f2dFontTexProvider::~f2dFontTexProvider()
{
	// 删除字体定义
	unordered_map<fCharW, f2dGlyphInfo*>::iterator i = m_Cache.begin();
	while(i != m_Cache.end())
	{
		FCYSAFEDEL(i->second);
		++i;
	}
	m_Cache.clear();

	FCYSAFEKILL(m_OrgTex);
}

fcyVec2 f2dFontTexProvider::readVec2Str(const std::wstring& Str)
{
	fcyVec2 tRet;
	if(2 != swscanf_s(Str.c_str(), L"%f,%f", &tRet.x, &tRet.y))
		throw fcyException("f2dFontTexProvider::readVec2Str", "String format error.");

	return tRet;
}

void f2dFontTexProvider::loadDefine(fcyXmlDocument& Xml)
{
	float tXScale = 1.f / m_OrgTex->GetWidth();
	float tYScale = 1.f / m_OrgTex->GetHeight();

	fcyXmlElement* pRoot = Xml.GetRootElement();

	if(pRoot->GetName() != L"f2dTexturedFont")
		throw fcyException("f2dFontTexProvider::loadDefine", "Invalid file, root node name not match.");
	
	fcyXmlElement* pMeasureNode = pRoot->GetFirstNode(L"Measure");
	if(!pMeasureNode)
		throw fcyException("f2dFontTexProvider::loadDefine", "Invalid file, node 'Measure' not found.");

	fcyXmlElement* pCharList = pRoot->GetFirstNode(L"CharList");

	// 读取度量值
	m_LineHeight = (float)_wtof(pMeasureNode->GetAttribute(L"LineHeight").c_str());
	m_Ascender = (float)_wtof(pMeasureNode->GetAttribute(L"Ascender").c_str());
	m_Descender = (float)_wtof(pMeasureNode->GetAttribute(L"Descender").c_str());

	// 读取字符表
	fcyXmlElementList tNodeList = pCharList->GetNodeByName(L"Item");
	fuInt tSubNodeCount = tNodeList.GetCount();
	for(fuInt i = 0; i<tSubNodeCount; ++i)
	{
		fcyXmlElement* pSub = tNodeList[i];

		const std::wstring& tChar = pSub->GetAttribute(L"Char");
		if(tChar.length() != 1)
			throw fcyException("f2dFontTexProvider::loadDefine", "Invalid file, invalid character in CharList.");
		
		f2dGlyphInfo* tInfo = new f2dGlyphInfo();
		tInfo->Advance = readVec2Str(pSub->GetAttribute(L"Advance"));
		tInfo->BrushPos = readVec2Str(pSub->GetAttribute(L"BrushPos"));
		tInfo->GlyphSize = readVec2Str(pSub->GetAttribute(L"Size"));
		tInfo->GlyphPos.a = readVec2Str(pSub->GetAttribute(L"Pos"));
		tInfo->GlyphPos.b = tInfo->GlyphPos.a + readVec2Str(pSub->GetAttribute(L"Size"));
		
		tInfo->GlyphPos.a.x *= tXScale;
		tInfo->GlyphPos.a.y *= tYScale;
		tInfo->GlyphPos.b.x *= tXScale;
		tInfo->GlyphPos.b.y *= tYScale;

		m_Cache[tChar[0]] = tInfo;
	}
}

fResult f2dFontTexProvider::QueryGlyph(f2dGraphics* pGraph, fCharU Character, f2dGlyphInfo* InfoOut)
{
	unordered_map<fCharW, f2dGlyphInfo*>::iterator i = m_Cache.find((fCharW)Character);

	if(i == m_Cache.end())
		return FCYERR_OBJNOTEXSIT;

	*InfoOut = *i->second;

	return FCYERR_OK;
}
