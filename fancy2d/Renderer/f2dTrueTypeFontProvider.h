#pragma once

#include <fcyRefObj.h>
#include <fcyIO/fcyStream.h>
#include "f2dRenderer.h"

#include <vector>
#include <unordered_map>

#include <ft2build.h> 
#include FT_FREETYPE_H

class f2dTrueTypeFontProvider :
	public fcyRefObjImpl<f2dFontProvider>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRendererImpl;
protected:
	static fuInt const TEXTURE_SIZE = 1024;
	static fuInt const INVALID_RECT = 0x7FFFFFFF;
	struct FreeTypeFontData
	{
		f2dStream* stream;
		FT_Open_Args ftargs;
		FT_StreamRec ftstream;
		FT_Face ftFace;
		fFloat ftLineHeight;
		fFloat ftAscender;
		fFloat ftDescender;
		fuInt bFallback;
	};
	struct Image2D
	{
		fuInt const width = TEXTURE_SIZE;
		fuInt const height = TEXTURE_SIZE;
		fuInt const pitch = TEXTURE_SIZE * sizeof(fcyColor);
		fcyColor data[TEXTURE_SIZE * TEXTURE_SIZE];
		inline fcyColor& pixel(fuInt x, fuInt y)
		{
			return data[y * 1024 + x];
		}
		Image2D();
	};
	struct GlyphCache2D
	{
		Image2D image;
		f2dTexture2D* texture = nullptr;
		fuInt pen_x = 0;
		fuInt pen_y = 0;
		fuInt pen_bottom = 0;
		fuInt dirty_l = INVALID_RECT;
		fuInt dirty_t = INVALID_RECT;
		fuInt dirty_r = INVALID_RECT;
		fuInt dirty_b = INVALID_RECT;
	};
	struct GlyphCacheInfo
	{
		// 来自 f2dGlyphInfo
		fuInt TextureIndex = 0; // 字形在哪个纹理上
		fcyRect GlyphPos;       // 字形在纹理上的uv坐标
		fcyVec2 GlyphSize;      // 字形大小
		fcyVec2 BrushPos;       // 笔触距离字形左上角坐标
		fcyVec2 Advance;        // 前进量
		// 私有域
		fCharU Character = 0;   // 当前的字符
	};
	struct FontCommonInfo
	{
		fFloat ftLineHeight = 0.0f;
		fFloat ftAscender = 0.0f;
		fFloat ftDescender = 0.0f;
	};
protected:
	f2dRenderDevice* m_pParent = nullptr;
	FontCommonInfo m_CommonInfo;
	std::vector<FreeTypeFontData> m_Font;
	std::vector<GlyphCache2D> m_Tex;
	std::unordered_map<fCharU, GlyphCacheInfo> m_Map;
private:
	void closeFonts();
	bool openFonts(f2dTrueTypeFontParam* fonts, fuInt count);
	bool addTexture();
	bool findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index);
	bool writeBitmapToCache(GlyphCacheInfo& info, FT_Bitmap& bitmap);
	GlyphCacheInfo* getGlyphCacheInfo(fCharU Char);
	bool renderCache(fCharU Char);
public: // 事件监听
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	fFloat GetLineHeight();
	fFloat GetAscender();
	fFloat GetDescender();
	fuInt GetCacheTextureCount();
	f2dTexture2D* GetCacheTexture(fuInt index);
	fResult CacheString(fcStrW String);
	fResult CacheStringU8(fcStr Text, fuInt Count);
	fResult QueryGlyph(f2dGraphics* pGraph, fCharU Character, f2dGlyphInfo* InfoOut);
	fInt GetCacheCount();
	fInt GetCacheTexSize();
	fResult Flush();
protected:
	f2dTrueTypeFontProvider(f2dRenderDevice* pParent, f2dTrueTypeFontParam* fonts, fuInt count);
	~f2dTrueTypeFontProvider();
};
