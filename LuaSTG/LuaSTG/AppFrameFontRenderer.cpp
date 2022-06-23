#include "AppFrame.h"
#include "utility/encoding.hpp"

namespace LuaSTGPlus
{
	// luastg plus interface
	
	constexpr int const TEXT_ALIGN_LEFT = 0x00;
	constexpr int const TEXT_ALIGN_CENTER = 0x01;
	constexpr int const TEXT_ALIGN_RIGHT = 0x02;

	constexpr int const TEXT_ALIGN_TOP = 0x00;
	constexpr int const TEXT_ALIGN_VCENTER = 0x04;
	constexpr int const TEXT_ALIGN_BOTTOM = 0x08;

	constexpr int const TEXT_FLAG_WORDBREAK = 0x10;

	bool AppFrame::RenderText(ResFont* p, wchar_t* strBuf, fcyRect rect, fcyVec2 scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)noexcept
	{
		using namespace Core;
		using namespace Core::Graphics;

		IGlyphManager* pGlyphManager = p->GetGlyphManager();
		
		// 准备渲染字体
		m_pTextRenderer->setGlyphManager(pGlyphManager);
		m_pTextRenderer->setScale(Vector2F(scale.x, scale.y));

		// 设置混合和颜色
		updateGraph2DBlendMode(p->GetBlendMode());
		m_pTextRenderer->setColor(p->GetBlendColor());
		
		// 第一次遍历计算要渲染多少行
		const wchar_t* pText = strBuf;
		int iLineCount = 1;
		float fLineWidth = 0.f;
		while (*pText)
		{
			bool bNewLine = false;
			if (*pText == L'\n')
				bNewLine = true;
			else
			{
				GlyphInfo tGlyphInfo{};
				if (pGlyphManager->getGlyph(*pText, &tGlyphInfo, true))
				{
					float adv = tGlyphInfo.advance.x * scale.x;
					if (bWordBreak && fLineWidth + adv > rect.GetWidth())  // 截断模式
					{
						if (pText == strBuf || *(pText - 1) == L'\n')
						{
							++pText;  // 防止一个字符都不渲染导致死循环
							if (*pText == L'\0')
								break;
						}
						bNewLine = true;
					}
					else
						fLineWidth += adv;
				}
			}
			if (bNewLine)
			{
				++iLineCount;
				fLineWidth = 0.f;
			}
			if (*pText != L'\0')
				++pText;
		}
		
		// 计算起笔位置
		float fTotalLineHeight = pGlyphManager->getLineHeight() * iLineCount * scale.y;
		fcyVec2 vRenderPos;
		switch (valign)
		{
		case ResFont::FontAlignVertical::Bottom:
			vRenderPos.y = rect.b.y + fTotalLineHeight;
			break;
		case ResFont::FontAlignVertical::Middle:
			vRenderPos.y = rect.a.y - rect.GetHeight() / 2.f + fTotalLineHeight / 2.f;
			break;
		case ResFont::FontAlignVertical::Top:
		default:
			vRenderPos.y = rect.a.y;
			break;
		}
		vRenderPos.x = rect.a.x;
		vRenderPos.y -= pGlyphManager->getAscender() * scale.y;
		
		// 逐行渲染文字
		wchar_t* pScanner = strBuf;
		wchar_t c = 0;
		bool bEOS = false;
		fLineWidth = 0.f;
		pText = pScanner;
		while (!bEOS)
		{
			// 寻找断句位置，换行、EOF、或者行溢出
			while (*pScanner != L'\0' && *pScanner != '\n')
			{
				GlyphInfo tGlyphInfo{};
				if (pGlyphManager->getGlyph(*pScanner, &tGlyphInfo, true))
				{
					float adv = tGlyphInfo.advance.x * scale.x;

					// 检查当前字符渲染后会不会导致行溢出
					if (bWordBreak && fLineWidth + adv > rect.GetWidth())
					{
						if (pScanner == pText)  // 防止一个字符都不渲染导致死循环
							++pScanner;
						break;
					}
					fLineWidth += adv;
				}
				++pScanner;
			}
			
			// 在断句位置写入\0
			c = *pScanner;
			if (c == L'\0')
				bEOS = true;
			else
				*pScanner = L'\0';
			
			// 渲染从pText~pScanner的文字
			std::string u8_str(std::move(utility::encoding::to_utf8(pText)));
			Vector2F ignore_;
			switch (halign)
			{
			case ResFont::FontAlignHorizontal::Right:
				m_pTextRenderer->drawText(u8_str, Vector2F(
					vRenderPos.x + rect.GetWidth() - fLineWidth,
					vRenderPos.y
				), &ignore_);
				break;
			case ResFont::FontAlignHorizontal::Center:
				m_pTextRenderer->drawText(u8_str, Vector2F(
					vRenderPos.x + rect.GetWidth() / 2.f - fLineWidth / 2.f,
					vRenderPos.y
				), &ignore_);
				break;
			case ResFont::FontAlignHorizontal::Left:
			default:
				m_pTextRenderer->drawText(u8_str, Vector2F(vRenderPos.x, vRenderPos.y), &ignore_);
				break;
			}

			// 恢复断句处字符
			*pScanner = c;
			fLineWidth = 0.f;
			if (c == L'\n')
				pText = ++pScanner;
			else
				pText = pScanner;
			
			// 移动y轴
			vRenderPos.y -= pGlyphManager->getLineHeight() * scale.y;
		}

		return true;
	}
	
	fcyVec2 AppFrame::CalcuTextSize(ResFont* p, const wchar_t* strBuf, fcyVec2 scale)noexcept
	{
		using namespace Core;
		using namespace Core::Graphics;

		IGlyphManager* pGlyphManager = p->GetGlyphManager();
		
		int iLineCount = 1;
		float fLineWidth = 0.f;
		float fMaxLineWidth = 0.f;
		while (*strBuf)
		{
			if (*strBuf == L'\n')
			{
				++iLineCount;
				fMaxLineWidth = std::max(fMaxLineWidth, fLineWidth);
				fLineWidth = 0.f;
			}
			else
			{
				GlyphInfo tGlyphInfo{};
				if (pGlyphManager->getGlyph(*strBuf, &tGlyphInfo, true))
					fLineWidth += tGlyphInfo.advance.x * scale.x;
			}
			++strBuf;
		}
		fMaxLineWidth = std::max(fMaxLineWidth, fLineWidth);
		
		return fcyVec2(fMaxLineWidth, iLineCount * pGlyphManager->getLineHeight() * scale.y);
	}
	
	bool AppFrame::RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)noexcept
	{
		fcyRefPointer<ResFont> p = m_ResourceMgr.FindSpriteFont(name);
		if (!p)
		{
			spdlog::error("[luastg] RenderText: 找不到字体资源'{}'", name);
			return false;
		}
		
		// 编码转换
		std::wstring s_TempStringBuf;
		try
		{
			s_TempStringBuf = std::move(utility::encoding::to_wide(str));
		}
		catch (const std::bad_alloc&)
		{
			spdlog::error("[luastg] RenderText: 内存不足");
			return false;
		}
		
		// 计算渲染位置
		fcyVec2 tSize = CalcuTextSize(p, s_TempStringBuf.c_str(), fcyVec2(scale, scale));
		switch (halign)
		{
		case ResFont::FontAlignHorizontal::Right:
			x -= tSize.x;
			break;
		case ResFont::FontAlignHorizontal::Center:
			x -= tSize.x / 2.f;
			break;
		case ResFont::FontAlignHorizontal::Left:
		default:
			break;
		}
		switch (valign)
		{
		case ResFont::FontAlignVertical::Bottom:
			y += tSize.y;
			break;
		case ResFont::FontAlignVertical::Middle:
			y += tSize.y / 2.f;
			break;
		case ResFont::FontAlignVertical::Top:
		default:
			break;
		}
		
		return RenderText(
			p,
			const_cast<wchar_t*>(s_TempStringBuf.data()),
			fcyRect(x, y, x + tSize.x, y - tSize.y),
			fcyVec2(scale, scale),
			halign,
			valign,
			false
			);
	}
	
	bool AppFrame::RenderTTF(const char* name, const char* str,
		float left, float right, float bottom, float top, float scale, int format, Core::Color4B c)noexcept
	{
		fcyRefPointer<ResFont> p = m_ResourceMgr.FindTTFFont(name);
		if (!p) {
			spdlog::error("[luastg] RenderTTF: 找不到字体资源'%m'", name);
			return false;
		}
		
		// 编码转换
		std::wstring s_TempStringBuf;
		try {
			s_TempStringBuf = std::move(utility::encoding::to_wide(str));
		}
		catch (const std::bad_alloc&) {
			spdlog::error("[luastg] RenderTTF: 内存不足");
			return false;
		}
		
		// 计算格式
		bool bWordBreak = false;
		ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Left;
		ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Top;
		
		if ((format & TEXT_ALIGN_CENTER) == TEXT_ALIGN_CENTER)
			halign = ResFont::FontAlignHorizontal::Center;
		else if ((format & TEXT_ALIGN_RIGHT) == TEXT_ALIGN_RIGHT)
			halign = ResFont::FontAlignHorizontal::Right;
		
		if ((format & TEXT_ALIGN_VCENTER) == TEXT_ALIGN_VCENTER)
			valign = ResFont::FontAlignVertical::Middle;
		else if ((format & TEXT_ALIGN_BOTTOM) == TEXT_ALIGN_BOTTOM)
			valign = ResFont::FontAlignVertical::Bottom;
		
		if ((format & TEXT_FLAG_WORDBREAK) == TEXT_FLAG_WORDBREAK)
			bWordBreak = true;
		
		p->SetBlendColor(c);
		return RenderText(
			p,
			const_cast<wchar_t*>(s_TempStringBuf.data()),
			fcyRect(left, top, right, bottom),
			fcyVec2(scale, scale) * 0.5f,  // 缩放系数=0.5
			halign,
			valign,
			bWordBreak
		);
	}
	
	// native interface
	
	bool AppFrame::FontRenderer_SetFontProvider(const char* name)
	{
		fcyRefPointer<ResFont> p = m_ResourceMgr.FindTTFFont(name);
		if (!p)
		{
			spdlog::error("[luastg] SetFontProvider: 找不到字体资源'{}'", name);
			return false;
		}
		m_pTextRenderer->setGlyphManager(p->GetGlyphManager());
		return true;
	}
	
	void AppFrame::FontRenderer_SetScale(const fcyVec2& s)
	{
		m_pTextRenderer->setScale(Core::Vector2F(s.x, s.y));
	}
	
	fcyRect AppFrame::FontRenderer_MeasureTextBoundary(const char* str, size_t len)
	{
		auto rc = m_pTextRenderer->getTextBoundary(Core::StringView(str, len));
		return fcyRect(rc.a.x, rc.a.y, rc.b.x, rc.b.y);
	}
	
	fcyVec2 AppFrame::FontRenderer_MeasureTextAdvance(const char* str, size_t len)
	{
		auto pt = m_pTextRenderer->getTextAdvance(Core::StringView(str, len));
		return fcyVec2(pt.x, pt.y);
	}
	
	bool AppFrame::FontRenderer_RenderText(const char* str, size_t len, fcyVec2& pos, const float z, const BlendMode blend, Core::Color4B const& color)
	{
		float const last_z = m_pTextRenderer->getZ();

		updateGraph2DBlendMode(blend);
		m_pTextRenderer->setZ(z);
		m_pTextRenderer->setColor(color);
		
		Core::Vector2F endpos;
		const bool result = m_pTextRenderer->drawText(Core::StringView(str, len), Core::Vector2F(pos.x, pos.y), &endpos);
		pos = fcyVec2(endpos.x, endpos.y);

		m_pTextRenderer->setZ(last_z);
		return result;
	}
	
	bool AppFrame::FontRenderer_RenderTextInSpace(const char* str, size_t len, fcyVec3& pos, const fcyVec3& rvec, const fcyVec3& dvec, const BlendMode blend, Core::Color4B const& color)
	{
		updateGraph2DBlendMode(blend);
		m_pTextRenderer->setColor(color);

		Core::Vector3F endpos;
		const bool result = m_pTextRenderer->drawTextInSpace(
			Core::StringView(str, len),
			Core::Vector3F(pos.x, pos.y, pos.z),
			Core::Vector3F(rvec.x, rvec.y, rvec.z),
			Core::Vector3F(dvec.x, dvec.y, dvec.z),
			&endpos);
		pos = fcyVec3(endpos.x, endpos.y, endpos.z);

		return result;
	}

	float AppFrame::FontRenderer_GetFontLineHeight()
	{
		auto* p = m_pTextRenderer->getGlyphManager();
		if (p) return p->getLineHeight();
		return 0.0f;
	}
	
	float AppFrame::FontRenderer_GetFontAscender()
	{
		auto* p = m_pTextRenderer->getGlyphManager();
		if (p) return p->getAscender();
		return 0.0f;
	}
	
	float AppFrame::FontRenderer_GetFontDescender()
	{
		auto* p = m_pTextRenderer->getGlyphManager();
		if (p) return p->getDescender();
		return 0.0f;
	}
};
