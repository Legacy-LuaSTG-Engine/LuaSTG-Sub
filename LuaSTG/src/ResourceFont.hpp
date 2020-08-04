#pragma once
#include "ResourceBase.hpp"

namespace LuaSTGPlus {
	// 纹理字体
	class ResFont :
		public Resource
	{
	public:
		enum class FontAlignHorizontal  // 水平对齐
		{
			Left,
			Center,
			Right
		};
		enum class FontAlignVertical  // 垂直对齐
		{
			Top,
			Middle,
			Bottom
		};

		// HGE纹理字体实现
		class HGEFont :
			public fcyRefObjImpl<f2dFontProvider>
		{
		public:
			static void ReadDefine(const std::wstring& data, std::unordered_map<wchar_t, f2dGlyphInfo>& out, std::wstring& tex);
		private:
			fcyRefPointer<f2dTexture2D> m_pTex;
			std::unordered_map<wchar_t, f2dGlyphInfo> m_Charset;
			float m_fLineHeight;
		public:
			fFloat GetLineHeight();
			fFloat GetAscender();
			fFloat GetDescender();
			f2dTexture2D* GetCacheTexture();
			fResult CacheString(fcStrW String);
			fResult QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut);
			fInt GetCacheCount() { return 0; }
			fInt GetCacheTexSize() { return 0; }
		public:
			HGEFont(std::unordered_map<wchar_t, f2dGlyphInfo>&& org, fcyRefPointer<f2dTexture2D> pTex);
		};
	private:
		fcyRefPointer<f2dFontProvider> m_pFontProvider;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		fcyColor m_BlendColor = fcyColor(0xFFFFFFFF);
	public:
		f2dFontProvider* GetFontProvider()LNOEXCEPT { return m_pFontProvider; }
		BlendMode GetBlendMode()const LNOEXCEPT { return m_BlendMode; }
		void SetBlendMode(BlendMode m)LNOEXCEPT { m_BlendMode = m; }
		fcyColor GetBlendColor()const LNOEXCEPT { return m_BlendColor; }
		void SetBlendColor(fcyColor c)LNOEXCEPT { m_BlendColor = c; }
	public:
		ResFont(const char* name, fcyRefPointer<f2dFontProvider> pFont);
	};
}
