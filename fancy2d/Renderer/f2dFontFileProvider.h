////////////////////////////////////////////////////////////////////////////////
/// @file  f2dFontFileProvider.h
/// @brief fancy2D 字体提供者
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <fcyRefObj.h>
#include <fcyIO/fcyStream.h>
#include "f2dRenderer.h"

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
	static const int s_Magin;    // 留边

	// 字体缓冲属性
	struct FontCacheInfo
	{
		// 原始属性
		fcyRect CacheSize;  // 缓冲位置

		// 字体属性
		fCharW Character;   // 当前的字符
		fcyRect UV;         // 字符UV
		fcyVec2 GlyphSize;  // 字形大小
		fcyVec2 BrushPos;   // 笔触位置
		fcyVec2 Advance;    // 前进量

		// 链表域
		FontCacheInfo* pPrev;
		FontCacheInfo* pNext;
	};
protected:
	f2dRenderDevice* m_pParent;

	// --- Freetype ---
	f2dStream* m_pStream;   // Font文件流
	FT_Open_Args m_Args;
	FT_StreamRec m_Stream;
	FT_Library m_FontLib;
	FT_Face m_Face;

	// --- 纹理缓冲区属性 ---
	fuInt m_TexSize;                // 纹理大小 1024或者2048 （还是用更大的尺寸好一些）
	fuInt m_CacheXCount;            // 缓冲横向数量
	fuInt m_CacheYCount;            // 缓冲纵向数量
	fcyVec2 m_PerGlyphSize;         // 单个字形大小
	FontCacheInfo* m_Cache;         // 缓冲区
	f2dTexture2D* m_CacheTex;       // 缓冲区纹理

	// --- 最近使用 ---
	FontCacheInfo* m_UsedNodeList;  // 使用中节点，保证为循环链表
	FontCacheInfo* m_FreeNodeList;  // 空闲节点，单向链表
	std::wstring m_CharInUsing;     // 用于恢复缓存

	// 字符表
	std::unordered_map<fCharW, FontCacheInfo*> m_Dict;
private: // freetype 函数
	// 实现freetype读取函数
	static unsigned long streamRead(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
	{
		f2dStream* pStream = (f2dStream*)stream->descriptor.pointer;

		pStream->Lock();

		fLen tRealReaded = 0;
		pStream->SetPosition(FCYSEEKORIGIN_BEG, offset);
		pStream->ReadBytes(buffer, count, &tRealReaded);

		pStream->Unlock();

		return (unsigned long)tRealReaded;
	}
	// 实现freetype关闭流操作
	static void streamClose(FT_Stream stream)
	{
		// 交给类析构函数处理
	}
protected: // 字体缓冲操作
	// 宽度单位到像素
	float widthSizeToPixel(int Size)
	{
		float tXScale = m_Face->size->metrics.x_scale / 65536.f;
		return (Size / 64.f) * tXScale;
	}
	// 高度单位到像素
	float heightSizeToPixel(int Size)
	{
		float tYScale = m_Face->size->metrics.y_scale / 65536.f;
		return (Size / 64.f) * tYScale;
	}

	void addUsedNode(FontCacheInfo* p);      // 加入最多使用节点
	void removeFreeNode(FontCacheInfo* p);   // 移除空闲节点p的连接
	void removeUsedNode(FontCacheInfo* p);   // 移除使用中节点p的连接
	f2dGlyphInfo getGlyphInfo(fCharW Char);  // 仅获得字形信息（不包括UV坐标）
	FontCacheInfo* getChar(fCharW Char);     // 获得字体
	bool makeCache(fuInt Size);                           // 创建缓冲区，存放XCount * YCount个文字
	bool renderCache(FontCacheInfo* pCache, fCharW Char); // 在缓冲区的pCache位置绘制字体Char
public: // 事件监听
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	fFloat GetLineHeight()
	{
		return heightSizeToPixel(m_Face->height);
	}
	fFloat GetAscender()
	{
		return heightSizeToPixel(m_Face->ascender);
	}
	fFloat GetDescender()
	{
		return heightSizeToPixel(m_Face->descender);
	}
	f2dTexture2D* GetCacheTexture()
	{
		return m_CacheTex;
	}
	fResult CacheString(fcStrW String);
	fResult QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut);
	fInt GetCacheCount() { return m_CacheXCount * m_CacheYCount; }
	fInt GetCacheTexSize() { return m_TexSize; }
protected:
	f2dFontFileProvider(f2dRenderDevice* pParent, f2dStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag);
	~f2dFontFileProvider();
};
