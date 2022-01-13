////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFontFileProvider.h
/// @brief fancy2D 字体提供者
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <fcyRefObj.h>
#include <fcyIO/fcyStream.h>
#include "f2dRenderer.h"

#include <vector>
#include <array>
#include <unordered_map>

#include <ft2build.h> 
#include FT_FREETYPE_H

////////////////////////////////////////////////////////////////////////////////
/// @brief 字体提供者实现
////////////////////////////////////////////////////////////////////////////////
class f2dFontFileProvider :
	public fcyRefObjImpl<f2dFontProvider>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRendererImpl;
protected:
	// Freetype 字体信息
	struct FontDataFreeType
	{
		f2dStream* stream;
		FT_Open_Args ftargs;
		FT_StreamRec ftstream;
		FT_Face ftFace;
		bool fallback;
	};
	struct FontDataCommon
	{
		fcyVec2 ftBBox;
		fFloat ftLineHeight;
		fFloat ftAscender;
		fFloat ftDescender;
	};
	struct FontData : public FontDataFreeType, public FontDataCommon {};
	// 字体缓冲属性
	struct FontCacheInfo
	{
		// 原始属性
		fcyRect CacheSize;  // 缓冲位置

		// 字体属性
		fCharU Character;   // 当前的字符
		fcyRect UV;         // 字符UV
		fcyVec2 GlyphSize;  // 字形大小
		fcyVec2 BrushPos;   // 笔触位置
		fcyVec2 Advance;    // 前进量

		// 链表域
		FontCacheInfo* pPrev;
		FontCacheInfo* pNext;
	};
protected:
	f2dRenderDevice* m_pParent = nullptr;
	
	// 传入的参数
	f2dFontProviderParam m_BaseParam;
	std::vector<f2dTrueTypeFontParam> m_Param;
	
	// FreeType
	FT_Library m_FontLib = nullptr;	// FreeType2 库
	std::vector<FontData> m_Fonts;	// FreeType2 字体集
	FontDataCommon m_FontsInfo;		// 公共字体信息（包围盒，行高，提升量、下降量）
	
	// 字形缓存基本信息
	fuInt m_TexSize			= 0; // 纹理大小
	fuInt m_CacheXCount		= 0; // 缓冲横向数量
	fuInt m_CacheYCount		= 0; // 缓冲纵向数量
	fuInt m_MaxGlyphWidth	= 0; // 单个字形宽度
	fuInt m_MaxGlyphHeight	= 0; // 单个字形高度
	
	// 字形缓存
	f2dTexture2D* m_CacheTex = nullptr;		// 缓冲区纹理
	fcyColor* m_CacheTexData = nullptr;		// 缓冲区纹理指针
	
	// 字形缓存数据结构
	fBool m_IsDirty = false;							// 是否需要Flush
	std::vector<FontCacheInfo> m_Cache;					// 缓冲区
	FontCacheInfo* m_UsedNodeList	= nullptr;			// 使用中节点，保证为循环链表
	FontCacheInfo* m_FreeNodeList	= nullptr;			// 空闲节点，单向链表
	std::unordered_map<fCharU, FontCacheInfo*> m_Dict;	// 字符表
	
	// 用过的标记
	std::array<uint8_t, 0x110000> m_UsedMark;
	uint32_t m_UsedCount = 0;
protected: // 字体操作
	bool openFonts(f2dFontProviderParam param, f2dTrueTypeFontParam* fonts, fuInt count);
	void closeFonts();
	bool findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index);
protected: // 字体缓冲链表操作
	void addFreeNode(FontCacheInfo* p);    // 加入最多空闲节点
	void removeFreeNode(FontCacheInfo* p); // 移除空闲节点p的连接
	void addUsedNode(FontCacheInfo* p);    // 加入最多使用节点
	void removeUsedNode(FontCacheInfo* p); // 移除使用中节点p的连接
protected: // 字体缓冲操作
	f2dGlyphInfo getGlyphInfo(fCharU Char);  // 仅获得字形信息（不包括UV坐标）
	FontCacheInfo* getChar(fCharU Char);     // 获得字体
	bool makeCacheMain();                                 // 创建缓冲区
	bool makeCache(fuInt Size);                           // 创建缓冲区，存放XCount * YCount个文字
	bool renderCache(FontCacheInfo* pCache, fCharU Char); // 在缓冲区的pCache位置绘制字体Char
public: // 事件监听
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	fFloat GetLineHeight();
	fFloat GetAscender();
	fFloat GetDescender();
	fuInt GetCacheTextureCount() { return 1; }
	f2dTexture2D* GetCacheTexture(fuInt index) { return m_CacheTex; }
	fResult CacheString(fcStrW String);
	fResult CacheStringU8(fcStr Text, fuInt Count);
	fResult QueryGlyph(f2dGraphics* pGraph, fCharU Character, f2dGlyphInfo* InfoOut);
	fInt GetCacheCount() { return m_CacheXCount * m_CacheYCount; }
	fInt GetCacheTexSize() { return m_TexSize; }
	fResult Flush();
protected:
	f2dFontFileProvider(f2dRenderDevice* pParent,
		f2dStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag);
	f2dFontFileProvider(f2dRenderDevice* pParent,
		fcyMemStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag);
	f2dFontFileProvider(f2dRenderDevice* pParent,
		f2dFontProviderParam param, f2dTrueTypeFontParam* fonts, fuInt count);
	~f2dFontFileProvider();
};
