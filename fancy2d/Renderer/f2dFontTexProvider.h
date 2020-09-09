////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFontTexProvider.h
/// @brief fancy2D 纹理字体提供者
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyParser/fcyXml.h"

#include "f2dRenderer.h"

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
/// @brief 纹理字体提供者实现
////////////////////////////////////////////////////////////////////////////////
class f2dFontTexProvider :
	public fcyRefObjImpl<f2dFontProvider>
{
	friend class f2dRendererImpl;
protected:
	// 原始纹理
	f2dTexture2D* m_OrgTex;
	
	// 字体度量值
	fFloat m_LineHeight;
	fFloat m_Ascender;
	fFloat m_Descender;

	// 字符表
	std::unordered_map<fCharW, f2dGlyphInfo*> m_Cache;
protected:
	fcyVec2 readVec2Str(const std::wstring& Str);
	void loadDefine(fcyXmlDocument& Xml);
public:
	fFloat GetLineHeight()
	{
		return m_LineHeight;
	}
	fFloat GetAscender()
	{
		return m_Ascender;
	}
	fFloat GetDescender()
	{
		return m_Descender;
	}
	f2dTexture2D* GetCacheTexture()
	{
		return m_OrgTex;
	}
	fResult CacheString(fcStrW String)
	{
		return FCYERR_NOTSUPPORT;
	}
	fResult QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut);
	fInt GetCacheCount() { return 0; }
	fInt GetCacheTexSize() { return 0; }
	fResult Flush() { return FCYERR_OK; }
protected:
	f2dFontTexProvider(f2dStream* pDefineFile, f2dTexture2D* pTex);
	f2dFontTexProvider(fcStrW pDefineFile, f2dTexture2D* pTex);
	~f2dFontTexProvider();
};
