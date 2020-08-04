#include "Renderer/f2dFontFileProvider.h"

#include <fcyException.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const int f2dFontFileProvider::s_Magin = 1;

f2dFontFileProvider::f2dFontFileProvider(f2dRenderDevice* pParent, f2dStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag)
	: m_pParent(pParent), m_pStream(pStream),
	m_CacheXCount(0), m_CacheYCount(0), m_Cache(NULL), m_CacheTex(NULL),
	m_UsedNodeList(NULL), m_FreeNodeList(NULL)
{
	// --- 初始化freetype ---
	FT_Error tErr = FT_Init_FreeType( &m_FontLib );
	if(tErr)
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "FT_Init_FreeType failed.");

	// --- 准备流 ---
	pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
	memset(&m_Args, 0, sizeof(m_Args));
	memset(&m_Stream, 0, sizeof(m_Stream));
	m_Args.flags = FT_OPEN_STREAM;
	m_Args.stream = &m_Stream;
	m_Stream.size = (unsigned long)m_pStream->GetLength();
	m_Stream.descriptor.pointer = m_pStream;
	m_Stream.read = streamRead;
	m_Stream.close = streamClose;
	m_pStream->AddRef();

	// --- 加载字体 ---
	tErr = FT_Open_Face(m_FontLib, &m_Args, FaceIndex, &m_Face);
	if(tErr)
	{
		FT_Done_FreeType(m_FontLib);

		FCYSAFEKILL(m_pStream);

		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "FT_New_Memory_Face failed.");
	}

	// --- 设置字体大小 ---
	FT_Set_Pixel_Sizes(m_Face, (FT_UInt)FontSize.x, (FT_UInt)FontSize.y);

	// --- 构建缓冲区 ---
	// 获得字体大小
	if (BBoxSize.x >= 0.01f && BBoxSize.y >= 0.01f) {
		m_PerGlyphSize.x = ceil(BBoxSize.x + s_Magin * 2.f);
		m_PerGlyphSize.y = ceil(BBoxSize.y + s_Magin * 2.f);
	}
	else {
		m_PerGlyphSize.x = ceil(widthSizeToPixel(abs(m_Face->bbox.xMax - m_Face->bbox.xMin)) + s_Magin * 2.f);
		m_PerGlyphSize.y = ceil(heightSizeToPixel(abs(m_Face->bbox.yMax - m_Face->bbox.yMin)) + s_Magin * 2.f);
	}

	/*
	// 计算行列的文字数
	if(1024.f / m_PerGlyphSize.x < 10.f || 1024.f / m_PerGlyphSize.y < 10.f)
	{
		// 不足10x10个字形
		if(2048.f / m_PerGlyphSize.x < 1.f || 2048.f / m_PerGlyphSize.y < 1.f)
		{
			// 不足一个字形
			FT_Done_Face(m_Face);
			FT_Done_FreeType(m_FontLib);

			FCYSAFEKILL(m_pStream);

			throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "Font size is too big.");
		}
		else
		{
			m_TexSize = 2048;
			m_CacheXCount = (int)(2048.f / m_PerGlyphSize.x);
			m_CacheYCount = (int)(2048.f / m_PerGlyphSize.y);
		}
	}
	else
	{
		m_TexSize = 1024;
		m_CacheXCount = (int)(1024.f / m_PerGlyphSize.x);
		m_CacheYCount = (int)(1024.f / m_PerGlyphSize.y);
	}
	*/

	// 计算行列的文字数
	{
		//贴图宽高
		int SizeTypeCount = 3;
		float TexSizeList[] = {
			512.0f,
			1024.0f,
			//2048.0f,
		};
		//每行列数量
		int LevelCount = 7;
		float CountLevel[] = {
			64.0f,
			32.0f,
			16.0f,
			8.0f,
			4.0f,
			2.0f,
			1.0f };
		//开始查询
		float s_size, n_level;
		bool check = false;
		for (int lv = 0; lv < LevelCount; lv++) {
			n_level = CountLevel[lv];//每行列数量
			for (int select = 0; select < SizeTypeCount; select++) {
				s_size = TexSizeList[select];//贴图宽高
				if (s_size / m_PerGlyphSize.x < n_level || s_size / m_PerGlyphSize.y < n_level) {
					continue;
				}
				else {
					m_TexSize = (int)s_size;
					m_CacheXCount = (int)(s_size / m_PerGlyphSize.x);
					m_CacheYCount = (int)(s_size / m_PerGlyphSize.y);
					check = true;
					break;
				}
			}
			//已经匹配到适合的每行列数量以及贴图大小
			if (check) {
				break;
			}
		}
		//没有匹配到，可能是字体过大？
		if (!check) {
			FT_Done_Face(m_Face);
			FT_Done_FreeType(m_FontLib);

			FCYSAFEKILL(m_pStream);

			throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "Font size is too big.");
		}
	}

	// 产生字体缓冲纹理
	if(!makeCache(m_TexSize))
	{
		FT_Done_Face(m_Face);
		FT_Done_FreeType(m_FontLib);

		FCYSAFEKILL(m_pStream);

		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "makeCache failed.");
	}

	// --- 绑定监听器 ---
	m_pParent->AttachListener(this);
}

f2dFontFileProvider::~f2dFontFileProvider()
{
	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_FontLib);

	// 释放字体缓冲
	FCYSAFEDELARR(m_Cache);
	FCYSAFEKILL(m_CacheTex);

	// 释放流
	FCYSAFEKILL(m_pStream);

	// 撤销监听器
	m_pParent->RemoveListener(this);
}

void f2dFontFileProvider::addUsedNode(FontCacheInfo* p)
{
	// 连接本节点
	if(m_UsedNodeList)
	{
		p->pPrev = m_UsedNodeList->pPrev;
		p->pNext = m_UsedNodeList;
	}
	else
	{
		p->pPrev = p->pNext = p;
	}

	// 插入本节点
	if(m_UsedNodeList)
	{
		m_UsedNodeList->pPrev->pNext = p;
		m_UsedNodeList->pPrev = p;
	}
	m_UsedNodeList = p;
}

void f2dFontFileProvider::removeFreeNode(FontCacheInfo* p)
{
	// 检查是否为表头
	if(p == m_FreeNodeList)
		m_FreeNodeList = p->pNext;

	// 移除本身的连接
	if(p->pPrev)
		p->pPrev->pNext = p->pNext;
	if(p->pNext)
		p->pNext->pPrev = p->pPrev;

	// 清空值
	p->pNext = p->pPrev = NULL;
}

void f2dFontFileProvider::removeUsedNode(FontCacheInfo* p)
{
	// 是否是首节点
	if(p == m_UsedNodeList)
		if(p->pNext == p)
			m_UsedNodeList = NULL;
		else
			m_UsedNodeList = p->pNext;

	// 移除自身的连接
	p->pNext->pPrev = p->pPrev;
	p->pPrev->pNext = p->pNext;

	p->pPrev = p->pNext = NULL;
}

f2dGlyphInfo f2dFontFileProvider::getGlyphInfo(fCharW Char) {
	// 加载文字到字形槽
	FT_Load_Char(m_Face, Char, FT_LOAD_DEFAULT);

	// 获得图像
	FT_Bitmap* tBitmap = &m_Face->glyph->bitmap;

	// 写入对应属性
	f2dGlyphInfo info;
	info.Advance = fcyVec2(m_Face->glyph->advance.x / 64.f, m_Face->glyph->advance.y / 64.f);
	info.BrushPos = fcyVec2(-(float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top);
	info.GlyphSize = fcyVec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows);

	return info;
}

f2dFontFileProvider::FontCacheInfo* f2dFontFileProvider::getChar(fCharW Char)
{
	// 先在字典里面寻找
	FontCacheInfo* pCache = m_Dict[Char];

	// 找到的话直接返回
	if(pCache)
	{
		// 找到UsedList中对应的位置，并删除节点，将其移到首位
		removeUsedNode(pCache);
		addUsedNode(pCache);

		return pCache;
	}

	// 否则检查是否有空位绘制字体
	if(m_FreeNodeList)
	{
		// 有空位
		pCache = m_FreeNodeList;

		// 从列表中移除
		removeFreeNode(pCache);

		// 绘制字体
		renderCache(pCache, Char);

		// 记录
		m_Dict[Char] = pCache;

		// 加入MRU列表
		addUsedNode(pCache);

		// 返回
		return pCache;
	}

	// 没有空位了
	{
		// 从UsedList的最后取出
		pCache = m_UsedNodeList->pPrev;

		// 清空对应的缓存数据
		m_Dict[pCache->Character] = NULL;

		// 移除
		removeUsedNode(pCache);

		// 绘制
		renderCache(pCache, Char);

		// 记录
		m_Dict[Char] = pCache;

		// 加入MRU列表
		addUsedNode(pCache);

		// 返回
		return pCache;
	}
}

bool f2dFontFileProvider::makeCache(fuInt Size)
{
	// 清除过往数据
	m_Dict.clear();
	FCYSAFEDELARR(m_Cache);
	FCYSAFEKILL(m_CacheTex);
	m_UsedNodeList = NULL;

	// 建立字体缓冲区
	if(FCYFAILED(m_pParent->CreateDynamicTexture(Size, Size, &m_CacheTex)))
		return false;
	// 清空缓冲区
	{
		fuInt tPitch = 0;
		fData tData = NULL;
		if(FCYFAILED(m_CacheTex->Lock(NULL, true, &tPitch, &tData)))
			return false;

		for(fuInt i = 0;i<m_CacheTex->GetHeight(); ++i)
		{
			fcyColor* pColor = (fcyColor*)tData;
			for(fuInt j = 0; j<m_CacheTex->GetWidth(); j++)
			{
				pColor[j].argb = 0x00000000;
			}
			tData += tPitch;
		}

		m_CacheTex->Unlock();
	}
	
	fuInt tCount = m_CacheXCount * m_CacheYCount;
	m_Cache = new FontCacheInfo[ tCount ];

	m_FreeNodeList = m_Cache;

	// 初始化信息
	int tCurIndex = 0;
	for(fuInt j = 0; j<m_CacheYCount; j++)
		for(fuInt i = 0; i<m_CacheXCount; ++i)
		{
			m_Cache[ tCurIndex ].CacheSize = fcyRect(
					i * m_PerGlyphSize.x, j * m_PerGlyphSize.y, 
					(i+1) * m_PerGlyphSize.x, (j+1) * m_PerGlyphSize.y
					);
			
			m_Cache[ tCurIndex ].Character = L'\0';
			
			tCurIndex++;
		}

	// 连接缓冲区
	m_Cache[0].pPrev = NULL;
	m_Cache[0].pNext = &m_Cache[1];
	m_Cache[tCount - 1].pNext = NULL;
	m_Cache[tCount - 1].pPrev = &m_Cache[ tCount - 2 ];
	for(fuInt i = 1; i<tCount - 1; ++i)
	{
		m_Cache[i].pPrev = &m_Cache[i - 1];
		m_Cache[i].pNext = &m_Cache[i + 1];
	}

	return true;
}

bool f2dFontFileProvider::renderCache(FontCacheInfo* pCache, fCharW Char)
{
	// 加载文字到字形槽并渲染
	FT_Load_Char( m_Face, Char, FT_LOAD_RENDER );

	// 获得图像
	FT_Bitmap* tBitmap = &m_Face->glyph->bitmap;

	// 写入对应属性
	pCache->Character = Char;
	pCache->Advance = fcyVec2(m_Face->glyph->advance.x / 64.f, m_Face->glyph->advance.y / 64.f);
	pCache->BrushPos = fcyVec2(-(float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top);
	pCache->UV = fcyRect(
		pCache->CacheSize.a + fcyVec2((float)s_Magin, (float)s_Magin),
		pCache->CacheSize.a + fcyVec2((float)tBitmap->width, (float)tBitmap->rows) + fcyVec2((float)s_Magin, (float)s_Magin)
		);
	pCache->GlyphSize = fcyVec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows); 
	
	// 转到UV坐标
	fcyVec2 tUVScale(1.f / (float)m_CacheTex->GetWidth(), 1.f / (float)m_CacheTex->GetHeight());
	pCache->UV.a.x *= tUVScale.x;
	pCache->UV.a.y *= tUVScale.y;
	pCache->UV.b.x *= tUVScale.x;
	pCache->UV.b.y *= tUVScale.y;

	// 拷贝到对应位置
	{
		fuInt tPitch = 0;
		fData tData = NULL;
		fData tSrcData = tBitmap->buffer;
		if(FCYFAILED(m_CacheTex->Lock(&pCache->CacheSize, false, &tPitch, &tData)))
			return false;
		
		int tHeight = (int)pCache->CacheSize.GetHeight();
		int tWidth = (int)pCache->CacheSize.GetWidth();

		for(int y = 0; y<tHeight; y++)
		{
			for(int x = 0; x<tWidth; x++)
			{
				fcyColor* pColor = (fcyColor*)tData;
				
				pColor[x].argb = 0x00000000;

				// 在字体数据中
				if (y >= s_Magin && y < s_Magin + tBitmap->rows)
				{
					if (x >= s_Magin && x < s_Magin + tBitmap->width)
						pColor[x].a = tSrcData[x - s_Magin];
				}	
			}

			if(y >= s_Magin && y < s_Magin + tBitmap->rows)
				tSrcData += tBitmap->pitch;
			tData += tPitch;
		}

		m_CacheTex->Unlock();
	}

	return true;
}

void f2dFontFileProvider::OnRenderDeviceLost()
{
	m_Dict.clear();
	
	// 保存所有已缓存的文字
	m_CharInUsing.clear();
	m_CharInUsing.reserve(m_CacheXCount * m_CacheYCount + 1);

	FontCacheInfo* pCache = m_UsedNodeList;
	do
	{
		if(pCache && pCache->Character != L'\0')
		{
			m_CharInUsing += pCache->Character;
			pCache = pCache->pNext;
		}
		else
			break;
	}
	while(pCache != m_UsedNodeList);
}

void f2dFontFileProvider::OnRenderDeviceReset()
{
	makeCache(m_TexSize);

	// 重新缓存
	CacheString(m_CharInUsing.c_str());
	m_CharInUsing.clear();
	m_CharInUsing.reserve(0);
}

fResult f2dFontFileProvider::CacheString(fcStrW String)
{
	fuInt tLen = wcslen(String);

	for(fuInt i = 0; i<tLen; ++i)
	{
		if(iswprint(String[i]))
			getChar(String[i]);
	}

	return FCYERR_OK;
}

fResult f2dFontFileProvider::QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut)
{
	if(Character != L'\t' && iswcntrl(Character))
		return FCYERR_INVAILDPARAM;
	if(!InfoOut)
		return FCYERR_INVAILDPARAM;

	if(Character == L'\t')
	{
		fResult tRet = QueryGlyph(pGraph, L' ', InfoOut);
		if(FCYOK(tRet))
			InfoOut->Advance.x *= 2.f;
		return tRet;
	}

	if(pGraph) {
		// 检查是否需要Flush
		if(m_FreeNodeList == NULL && !m_Dict[Character])
			pGraph->Flush();

		// 取出文字，绘制到纹理
		FontCacheInfo* pCache = getChar(Character);

		InfoOut->Advance = pCache->Advance;
		InfoOut->BrushPos = pCache->BrushPos;
		InfoOut->GlyphPos = pCache->UV;
		InfoOut->GlyphSize = pCache->GlyphSize;
	}
	else {
		// 只是查询信息，先在字典里面寻找（这部分代码复制自getChar）
		FontCacheInfo* pCache = m_Dict[Character];

		// 找到的话直接返回
		if (pCache) {
			// 找到UsedList中对应的位置，并删除节点，将其移到首位
			removeUsedNode(pCache);
			addUsedNode(pCache);

			// 复制信息
			InfoOut->Advance = pCache->Advance;
			InfoOut->BrushPos = pCache->BrushPos;
			InfoOut->GlyphPos = pCache->UV;
			InfoOut->GlyphSize = pCache->GlyphSize;
		}
		else {
			// 在字体里查找
			f2dGlyphInfo info = getGlyphInfo(Character);
			*InfoOut = info;
		}
	}

	return FCYERR_OK;
}
