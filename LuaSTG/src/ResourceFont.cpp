#include "ResourceFont.hpp"
#include "AppFrame.h"
#include "Utility.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace std;

namespace LuaSTGPlus {
	void ResFont::HGEFont::ReadDefine(const std::wstring& data, std::unordered_map<wchar_t, f2dGlyphInfo>& out, std::wstring& tex)
	{
		out.clear();
		tex.clear();

		std::vector<std::wstring> tLines;
		fcyStringHelper::StringSplit(data, L"\n", true, tLines);
		for (auto& i : tLines)
		{
			i = fcyStringHelper::Trim(i);
		}

		// 第一行必须是HGEFONT
		if (tLines.size() <= 1 || tLines[0] != L"[HGEFONT]")
			throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");

		for (size_t i = 1; i < tLines.size(); ++i)
		{
			wstring& tLine = tLines[i];
			if (tLine.size() == 0)
				continue;

			wstring::size_type tPos;
			if (string::npos == (tPos = tLine.find_first_of(L"=")))
				throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
			wstring tKey = tLine.substr(0, tPos);
			wstring tValue = tLine.substr(tPos + 1, tLine.size() - tPos - 1);
			if (tKey == L"Bitmap")
				tex = tValue;
			else if (tKey == L"Char")
			{
				wchar_t c;
				int c_hex;
				float x, y, w, h, left_offset, right_offset;
				if (7 != swscanf(tValue.c_str(), L"\"%c\",%f,%f,%f,%f,%f,%f", &c, &x, &y, &w, &h, &left_offset, &right_offset))
				{
					if (7 != swscanf(tValue.c_str(), L"%X,%f,%f,%f,%f,%f,%f", &c_hex, &x, &y, &w, &h, &left_offset, &right_offset))
						throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
					c = static_cast<wchar_t>(c_hex);
				}

				// 计算到f2d字体偏移量
				f2dGlyphInfo tInfo = {
					fcyRect(x, y, x + w, y + h),
					fcyVec2(w, h),
					fcyVec2(-left_offset, h),
					fcyVec2(w + left_offset + right_offset, 0)
				};
				if (out.find(c) != out.end())
					throw fcyException("ResFont::HGEFont::readDefine", "Duplicated character defination.");
				out.emplace(c, tInfo);
			}
			else
				throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
		}

		if (tex.empty())
			throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
	}

	ResFont::HGEFont::HGEFont(std::unordered_map<wchar_t, f2dGlyphInfo>&& org, fcyRefPointer<f2dTexture2D> pTex)
		: m_Charset(std::move(org)), m_pTex(pTex)
	{
		// 计算最高行作为LineHeight
		m_fLineHeight = 0;
		for (auto i = m_Charset.begin(); i != m_Charset.end(); ++i)
			m_fLineHeight = ::max(m_fLineHeight, i->second.GlyphSize.y);

		// 修正纹理坐标
		for (auto& i : m_Charset)
		{
			i.second.GlyphPos.a.x /= pTex->GetWidth();
			i.second.GlyphPos.b.x /= pTex->GetWidth();
			i.second.GlyphPos.a.y /= pTex->GetHeight();
			i.second.GlyphPos.b.y /= pTex->GetHeight();
		}
	}

	fFloat ResFont::HGEFont::GetLineHeight()
	{
		return m_fLineHeight + 1.f;
	}

	fFloat ResFont::HGEFont::GetAscender()
	{
		return m_fLineHeight;
	}

	fFloat ResFont::HGEFont::GetDescender()
	{
		return 0.f;
	}

	f2dTexture2D* ResFont::HGEFont::GetCacheTexture()
	{
		return m_pTex;
	}

	fResult ResFont::HGEFont::CacheString(fcStrW String)
	{
		return FCYERR_OK;  // 纹理字体不需要实现CacheString
	}

	fResult ResFont::HGEFont::QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut)
	{
		unordered_map<wchar_t, f2dGlyphInfo>::iterator i = m_Charset.find(Character);

		if (i == m_Charset.end())
			return FCYERR_OBJNOTEXSIT;

		*InfoOut = i->second;

		return FCYERR_OK;
	}

	ResFont::ResFont(const char* name, fcyRefPointer<f2dFontProvider> pFont)
		: Resource(ResourceType::SpriteFont, name), m_pFontProvider(pFont)
	{
	}
}
