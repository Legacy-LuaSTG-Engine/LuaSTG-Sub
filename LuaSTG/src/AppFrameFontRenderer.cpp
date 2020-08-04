#include "AppFrame.h"

namespace LuaSTGPlus {
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

	bool AppFrame::FontRenderer_DrawTextW2(const char* str, const fcyVec2& pos, const float z, const BlendMode blend, const fcyColor& color) {
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

		const bool result = FCYERR_OK == m_FontRenderer->DrawTextW2(m_Graph2D, g_wbuffer.c_str(), pos);

		m_FontRenderer->SetZ(lastz);
		return result;
	}
};
