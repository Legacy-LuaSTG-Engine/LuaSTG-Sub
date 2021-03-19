#include "AppFrame.h"
#include "UnicodeStringEncoding.h"

namespace LuaSTGPlus {
	// deug
	
#ifdef LSHOWFONTBASELINE
	class FontBaseLineDebugHelper :
		public f2dFontRendererListener
	{
	private:
		f2dGraphics2D* m_pGraph2D;
		f2dGeometryRenderer* m_pGRenderer;
	protected:
		fBool OnGlyphBeginDraw(fuInt Index, fCharW Character, fcyVec2& DrawPos, fcyVec2& Adv)
		{
			m_pGRenderer->SetPenColor(0, fcyColor(0xFF00FFFF));
			m_pGRenderer->SetPenColor(1, fcyColor(0xFF00FFFF));
			m_pGRenderer->SetPenColor(2, fcyColor(0xFF00FFFF));
			m_pGRenderer->SetPenColor(3, fcyColor(0xFF00FFFF));
			m_pGRenderer->SetPenSize(3.f);
			m_pGRenderer->DrawCircle(m_pGraph2D, DrawPos, 2.f, 12);
			m_pGRenderer->SetPenColor(0, fcyColor(0xFF00FF00));
			m_pGRenderer->SetPenColor(1, fcyColor(0xFF00FF00));
			m_pGRenderer->SetPenColor(2, fcyColor(0xFF00FF00));
			m_pGRenderer->SetPenColor(3, fcyColor(0xFF00FF00));
			m_pGRenderer->DrawLine(m_pGraph2D, DrawPos, DrawPos + Adv);
			return true;
		}
		void OnGlyphCalcuCoord(f2dGraphics2DVertex pVerts[]) { }
	public:
		FontBaseLineDebugHelper(f2dGraphics2D* G, f2dGeometryRenderer* GR, fcyRect BaseRect)
			: m_pGraph2D(G), m_pGRenderer(GR)
		{
			m_pGRenderer->SetPenColor(0, fcyColor(0xFFFF0000));
			m_pGRenderer->SetPenColor(1, fcyColor(0xFFFF0000));
			m_pGRenderer->SetPenColor(2, fcyColor(0xFFFF0000));
			m_pGRenderer->SetPenColor(3, fcyColor(0xFFFF0000));
			m_pGRenderer->SetPenSize(2.f);
			m_pGRenderer->DrawRectangle(G, BaseRect);
		}
	};
#endif
	
	// luastg plus interface
	
	bool AppFrame::RenderText(ResFont* p, wchar_t* strBuf, fcyRect rect, fcyVec2 scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)LNOEXCEPT
	{
		if (m_GraphType != GraphicsType::Graph2D) {
			LERROR("RenderText: 只有2D渲染器可以执行该方法");
			return false;
		}
		
		f2dFontProvider* pFontProvider = p->GetFontProvider();
		
		// 准备渲染字体
		m_FontRenderer->SetFontProvider(pFontProvider);
		m_FontRenderer->SetScale(scale);
	#ifdef LSHOWFONTBASELINE
		FontBaseLineDebugHelper tDebugger(m_Graph2D, m_GRenderer, rect);
		m_FontRenderer->SetListener(&tDebugger);
	#endif
		
		// 设置混合和颜色
		updateGraph2DBlendMode(p->GetBlendMode());
		m_FontRenderer->SetColor(p->GetBlendColor());
		
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
				f2dGlyphInfo tGlyphInfo;
				if (FCYOK(pFontProvider->QueryGlyph(NULL, *pText, &tGlyphInfo)))
				{
					float adv = tGlyphInfo.Advance.x * scale.x;
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
		float fTotalLineHeight = pFontProvider->GetLineHeight() * iLineCount * scale.y;
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
		vRenderPos.y -= pFontProvider->GetAscender() * scale.y;
		
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
				f2dGlyphInfo tGlyphInfo;
				if (FCYOK(pFontProvider->QueryGlyph(NULL, *pScanner, &tGlyphInfo)))
				{
					float adv = tGlyphInfo.Advance.x * scale.x;

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
			switch (halign)
			{
			case ResFont::FontAlignHorizontal::Right:
				m_FontRenderer->DrawTextW2(m_Graph2D, pText,
					fcyVec2(vRenderPos.x + rect.GetWidth() - fLineWidth, vRenderPos.y));
				break;
			case ResFont::FontAlignHorizontal::Center:
				m_FontRenderer->DrawTextW2(m_Graph2D, pText,
					fcyVec2(vRenderPos.x + rect.GetWidth() / 2.f - fLineWidth / 2.f, vRenderPos.y));
				break;
			case ResFont::FontAlignHorizontal::Left:
			default:
				m_FontRenderer->DrawTextW2(m_Graph2D, pText, vRenderPos);
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
			vRenderPos.y -= p->GetFontProvider()->GetLineHeight() * scale.y;
		}

	#ifdef LSHOWFONTBASELINE
		m_FontRenderer->SetListener(nullptr);
	#endif
		return true;
	}

	fcyVec2 AppFrame::CalcuTextSize(ResFont* p, const wchar_t* strBuf, fcyVec2 scale)LNOEXCEPT
	{
		if (m_GraphType != GraphicsType::Graph2D)
		{
			LERROR("RenderText: 只有2D渲染器可以执行该方法");
			return fcyVec2();
		}

		f2dFontProvider* pFontProvider = p->GetFontProvider();

		int iLineCount = 1;
		float fLineWidth = 0.f;
		float fMaxLineWidth = 0.f;
		while (*strBuf)
		{
			if (*strBuf == L'\n')
			{
				++iLineCount;
				fMaxLineWidth = max(fMaxLineWidth, fLineWidth);
				fLineWidth = 0.f;
			}
			else
			{
				f2dGlyphInfo tGlyphInfo;
				if (FCYOK(pFontProvider->QueryGlyph(NULL, *strBuf, &tGlyphInfo)))
					fLineWidth += tGlyphInfo.Advance.x * scale.x;
			}
			++strBuf;
		}
		fMaxLineWidth = max(fMaxLineWidth, fLineWidth);

		return fcyVec2(fMaxLineWidth, iLineCount * pFontProvider->GetLineHeight() * scale.y);
	}

	LNOINLINE bool AppFrame::RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)LNOEXCEPT
	{
		fcyRefPointer<ResFont> p = m_ResourceMgr.FindSpriteFont(name);
		if (!p)
		{
			LERROR("RenderText: 找不到文字资源'%m'", name);
			return false;
		}

		// 编码转换
		static std::wstring s_TempStringBuf;
		try
		{
			Utf8ToUtf16(str, s_TempStringBuf);
		}
		catch (const std::bad_alloc&)
		{
			LERROR("RenderText: 内存不足");
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
	
	LNOINLINE bool AppFrame::RenderTTF(const char* name, const char* str,
		float left, float right, float bottom, float top, float scale, int format, fcyColor c)LNOEXCEPT
	{
		fcyRefPointer<ResFont> p = m_ResourceMgr.FindTTFFont(name);
		if (!p) {
			LERROR("RenderTTF: 找不到文字资源'%m'", name);
			return false;
		}
		
		// 编码转换
		static std::wstring s_TempStringBuf;
		try {
			Utf8ToUtf16(str, s_TempStringBuf);
		}
		catch (const std::bad_alloc&) {
			LERROR("RenderTTF: 内存不足");
			return false;
		}
		
		// 计算格式
		bool bWordBreak = false;
		ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Left;
		ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Top;
		
		if ((format & DT_CENTER) == DT_CENTER)
			halign = ResFont::FontAlignHorizontal::Center;
		else if ((format & DT_RIGHT) == DT_RIGHT)
			halign = ResFont::FontAlignHorizontal::Right;

		if ((format & DT_VCENTER) == DT_VCENTER)
			valign = ResFont::FontAlignVertical::Middle;
		else if ((format & DT_BOTTOM) == DT_BOTTOM)
			valign = ResFont::FontAlignVertical::Bottom;

		if ((format & DT_WORDBREAK) == DT_WORDBREAK)
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
	
	bool AppFrame::FontRenderer_SetFontProvider(const char* name) {
		fcyRefPointer<ResFont> p = m_ResourceMgr.FindTTFFont(name);
		if (!p)
		{
			LERROR("SetFontProvider: 找不到文字资源'%m'", name);
			return false;
		}
		m_FontRenderer->SetFontProvider(p->GetFontProvider());
		return true;
	}
	
	void AppFrame::FontRenderer_SetFlipType(const F2DSPRITEFLIP t) {
		m_FontRenderer->SetFlipType(t);
	}
	
	void AppFrame::FontRenderer_SetScale(const fcyVec2& s) {
		m_FontRenderer->SetScale(s);
	}
	
	static std::wstring g_wbuffer;
	
	fcyRect AppFrame::FontRenderer_MeasureString(const char* str, bool strict) {
		try {
			g_wbuffer = fcyStringHelper::MultiByteToWideChar(str, CP_UTF8);
		}
		catch (...) {
			return fcyRect();
		}
		return m_FontRenderer->MeasureString(g_wbuffer.c_str(), strict);
	}
	
	float AppFrame::FontRenderer_MeasureStringWidth(const char* str) {
		try {
			g_wbuffer = fcyStringHelper::MultiByteToWideChar(str, CP_UTF8);
		}
		catch (...) {
			return 0.0f;
		}
		return m_FontRenderer->MeasureStringWidth(g_wbuffer.c_str());
	}
	
	bool AppFrame::FontRenderer_DrawTextW2(const char* str, fcyVec2& pos, const float z, const BlendMode blend, const fcyColor& color) {
		if (m_GraphType != GraphicsType::Graph2D) {
			LERROR("DrawText: 只有2D渲染器可以执行该方法");
			return false;
		}
		
		try {
			g_wbuffer = fcyStringHelper::MultiByteToWideChar(str, CP_UTF8);
		}
		catch (...) {
			return false;
		}
		
		const float lastz = m_FontRenderer->GetZ();
		m_FontRenderer->SetZ(z);

		updateGraph2DBlendMode(blend);
		m_FontRenderer->SetColor(color);

		const bool result = FCYERR_OK == m_FontRenderer->DrawTextW2(m_Graph2D, g_wbuffer.c_str(), g_wbuffer.length(), pos, &pos);

		m_FontRenderer->SetZ(lastz);
		return result;
	}
};
