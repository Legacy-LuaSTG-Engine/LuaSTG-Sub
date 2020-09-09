#include "Renderer/f2dFontFileProvider.h"
#include <fcyException.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

uint32_t calTextureSize(uint32_t width, uint32_t height, uint32_t& xcnt, uint32_t& ycnt) {
	// 傻逼吗，没事搞0大小的字形，屌你妈的
	width = (width > 0) ? width : 1;
	height = (height > 0) ? height : 1;
	// 查找
	const uint32_t tex_size[] = {
		64,
		128,
		256,
		512,
		1024,
	};
	const uint32_t tex_size_array_size = sizeof(tex_size) / sizeof(tex_size[0]);
	const float f_width = (float)(width + 1);
	const float f_height = (float)(height + 1);
	for (uint32_t i = 0; i < tex_size_array_size; i += 1)
	{
		float f_tex_wh = (float)(tex_size[i] - 1);
		uint32_t x_cnt = (uint32_t)floor(f_tex_wh / f_width);
		uint32_t y_cnt = (uint32_t)floor(f_tex_wh / f_height);
		if ((x_cnt * y_cnt) >= 1024 || i >= (tex_size_array_size - 1)) {
			xcnt = x_cnt;
			ycnt = y_cnt;
			return tex_size[i];
		}
	}
	// 太大了
	xcnt = 1;
	ycnt = 1;
	return 1024;
}

////////////////////////////////////////////////////////////////////////////////

// 引擎实现接口

f2dFontFileProvider::f2dFontFileProvider(f2dRenderDevice* pParent,
	f2dStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag)
{
	m_pParent = pParent;
	
	// --- 初始化freetype ---
	FT_Error tErr = FT_Init_FreeType( &m_FontLib );
	if(tErr)
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "FT_Init_FreeType failed.");
	
	// --- 准备流 ---
	m_pStream = pStream;
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
	m_pStream->AddRef();
	
	memset(&m_Args, 0, sizeof(m_Args));
	memset(&m_Stream, 0, sizeof(m_Stream));
	m_Args.flags = FT_OPEN_STREAM;
	m_Args.stream = &m_Stream;
	m_Stream.size = (unsigned long)m_pStream->GetLength();
	m_Stream.descriptor.pointer = m_pStream;
	m_Stream.read = streamRead;
	m_Stream.close = streamClose;
	
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
	
	// 计算最大字形大小
	if (BBoxSize.x >= 1.0f && BBoxSize.y >= 1.0f) {
		// 这个参数平时没卵用，专门用来杀了思源系列字体的妈，没事搞超大字形包围盒，导致字形缓存TMD空间压根不够用
		m_MaxGlyphWidth = (fuInt)ceil(BBoxSize.x);
		m_MaxGlyphHeight = (fuInt)ceil(BBoxSize.y);
	}
	else {
		m_MaxGlyphWidth = (fuInt)ceil(widthSizeToPixel(abs(m_Face->bbox.xMax - m_Face->bbox.xMin)));
		m_MaxGlyphHeight = (fuInt)ceil(heightSizeToPixel(abs(m_Face->bbox.yMax - m_Face->bbox.yMin)));
	}
	// 计算所需的纹理大小
	m_TexSize = calTextureSize(m_MaxGlyphWidth, m_MaxGlyphHeight, m_CacheXCount, m_CacheYCount);
	// 处理一些异常情况（比如字形比纹理大）
	m_MaxGlyphWidth = (m_MaxGlyphWidth > m_TexSize) ? m_TexSize : m_MaxGlyphWidth;
	m_MaxGlyphHeight = (m_MaxGlyphHeight > m_TexSize) ? m_TexSize : m_MaxGlyphHeight;
	
	// 产生字体缓存
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

f2dFontFileProvider::f2dFontFileProvider(f2dRenderDevice* pParent,
	fcyMemStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag)
{
	m_pParent = pParent;
	
	// --- 初始化freetype ---
	FT_Error tErr = FT_Init_FreeType( &m_FontLib );
	if(tErr)
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "FT_Init_FreeType failed.");
	
	// --- 准备流 ---
	m_pStream = (f2dStream*)pStream;
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);
	m_pStream->AddRef();
	
	memset(&m_Args, 0, sizeof(m_Args));
	m_Args.flags = FT_OPEN_MEMORY;
	m_Args.memory_base = pStream->GetInternalBuffer();
	m_Args.memory_size = pStream->GetLength();
	
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
	
	// 计算最大字形大小
	if (BBoxSize.x >= 1.0f && BBoxSize.y >= 1.0f) {
		// 这个参数平时没卵用，专门用来杀了思源系列字体的妈，没事搞超大字形包围盒，导致字形缓存TMD空间压根不够用
		m_MaxGlyphWidth = (fuInt)ceil(BBoxSize.x);
		m_MaxGlyphHeight = (fuInt)ceil(BBoxSize.y);
	}
	else {
		m_MaxGlyphWidth = (fuInt)ceil(widthSizeToPixel(abs(m_Face->bbox.xMax - m_Face->bbox.xMin)));
		m_MaxGlyphHeight = (fuInt)ceil(heightSizeToPixel(abs(m_Face->bbox.yMax - m_Face->bbox.yMin)));
	}
	// 计算所需的纹理大小
	m_TexSize = calTextureSize(m_MaxGlyphWidth, m_MaxGlyphHeight, m_CacheXCount, m_CacheYCount);
	// 处理一些异常情况（比如字形比纹理大）
	m_MaxGlyphWidth = (m_MaxGlyphWidth > m_TexSize) ? m_TexSize : m_MaxGlyphWidth;
	m_MaxGlyphHeight = (m_MaxGlyphHeight > m_TexSize) ? m_TexSize : m_MaxGlyphHeight;
	
	// 产生字体缓存
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
	// 释放纹理
	if (m_CacheTex) {
		m_CacheTex->Unlock();
	}
	FCYSAFEKILL(m_CacheTex);
	
	// 关闭FreeType
	FT_Done_Face(m_Face);
	FT_Done_FreeType(m_FontLib);
	FCYSAFEKILL(m_pStream);

	// 撤销监听器
	m_pParent->RemoveListener(this);
}

void f2dFontFileProvider::OnRenderDeviceLost()
{
	m_Dict.clear();
	m_UsedMark.fill(0);
	m_UsedCount = 0;
}

void f2dFontFileProvider::OnRenderDeviceReset()
{
	makeCache(m_TexSize);
}

// MRU 链表管理

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

// 字形管理

f2dGlyphInfo f2dFontFileProvider::getGlyphInfo(fCharW Char) {
	// 加载文字到字形槽
	FT_Load_Char(m_Face, Char, FT_LOAD_DEFAULT);
	FT_Bitmap* tBitmap = &m_Face->glyph->bitmap;

	// 写入对应属性
	f2dGlyphInfo info;
	info.Advance = fcyVec2(m_Face->glyph->advance.x / 64.f, m_Face->glyph->advance.y / 64.f);
	info.BrushPos = fcyVec2((float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top);
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
	// 清除
	FCYSAFEKILL(m_CacheTex);
	m_Cache.clear();
	m_FreeNodeList = nullptr;
	m_UsedNodeList = nullptr;
	m_Dict.clear();
	
	// 建立字形缓存纹理
	if(FCYFAILED(m_pParent->CreateDynamicTexture(Size, Size, &m_CacheTex)))
	{
		return false;
	}
	else
	{
		// 生成空数据
		std::vector<fcyColor> _data;
		_data.resize(Size * Size);
		auto _byte_size = sizeof(fcyColor) * Size * Size;
		memset(_data.data(), 0, _byte_size);
		// 复制
		fuInt tPitch = 0;
		fData tData = NULL;
		if(FCYFAILED(m_CacheTex->Lock(NULL, false, &tPitch, &tData)))
			return false;
		memcpy(tData, _data.data(), _byte_size);
		m_CacheTexData = (fcyColor*)tData;
		//m_CacheTex->Unlock(); // 别问为什么注释掉，反正就是从头lock到尾，lock他妈的，提升性能
	}
	
	// 创建缓冲链表
	fuInt tCount = m_CacheXCount * m_CacheYCount;
	m_Cache.resize(tCount);
	// 初始化缓冲链表内容
	m_FreeNodeList = &m_Cache[0]; // 空闲头
	fuInt tCurIndex = 0;
	fuInt x = 1; // 纹理的U坐标（像素坐标）
	fuInt y = 1; // 纹理的V坐标（像素坐标）
	for(fuInt j = 0; j < m_CacheYCount; j += 1)
	{
		x = 1; // 回到行首
		for(fuInt i = 0; i < m_CacheXCount; i += 1)
		{
			auto& slot = m_Cache[tCurIndex];
			slot.CacheSize = fcyRect(
				x,
				y,
				x + m_MaxGlyphWidth,
				y + m_MaxGlyphHeight
			);
			slot.UV = fcyRect(
				fcyVec2(
					(float)x / (float)m_TexSize,
					(float)y / (float)m_TexSize
				),
				fcyVec2(
					(float)(x + m_MaxGlyphWidth) / (float)m_TexSize,
					(float)(y + m_MaxGlyphHeight) / (float)m_TexSize
				)
			);
			slot.Character = L'\0';
			
			tCurIndex += 1; // 下一个
			x += (m_MaxGlyphWidth + 1); // 下一列
		}
		y += (m_MaxGlyphHeight + 1); // 下一行
	}
	// 连接缓冲链表
	if (tCount > 1) {
		m_Cache[0].pPrev = nullptr;
		m_Cache[0].pNext = &m_Cache[1];
		m_Cache[tCount - 1].pNext = nullptr;
		m_Cache[tCount - 1].pPrev = &m_Cache[tCount - 2];
		for(fuInt i = 1; i < (tCount - 1); i += 1)
		{
			m_Cache[i].pPrev = &m_Cache[i - 1];
			m_Cache[i].pNext = &m_Cache[i + 1];
		}
	}
	else
	{
		m_Cache[0].pPrev = nullptr;
		m_Cache[0].pNext = nullptr;
	}
	
	return true;
}

bool f2dFontFileProvider::renderCache(FontCacheInfo* pCache, fCharW Char)
{
	// 加载文字到字形槽并渲染
	FT_Load_Char(m_Face, Char, FT_LOAD_RENDER);
	FT_Bitmap& tBitmap = m_Face->glyph->bitmap;
	
	// 写入对应属性
	pCache->Character = Char;
	pCache->Advance = fcyVec2(m_Face->glyph->advance.x / 64.f, m_Face->glyph->advance.y / 64.f);
	pCache->BrushPos = fcyVec2((float)m_Face->glyph->bitmap_left, (float)m_Face->glyph->bitmap_top);
	pCache->GlyphSize = fcyVec2((float)m_Face->glyph->bitmap.width, (float)m_Face->glyph->bitmap.rows); 
	pCache->UV.b = pCache->UV.a + fcyVec2(
		(float)tBitmap.width / (float)m_TexSize,
		(float)tBitmap.rows / (float)m_TexSize
	);
	
	// 拷贝到上传缓冲区
	{
		fuInt uOffset = (fuInt)pCache->CacheSize.a.y * m_TexSize + (fuInt)pCache->CacheSize.a.x;
		fcyColor* tPixel = m_CacheTexData + uOffset;
		
		fByte* tBuffer = tBitmap.buffer;
		for(int y = 0; y < m_MaxGlyphHeight; y += 1)
		{
			for(int x = 0; x < m_MaxGlyphWidth; x += 1)
			{
				tPixel[x].argb = 0x00000000;
				// 在字体数据中
				if (x < tBitmap.width && y < tBitmap.rows)
				{
					tPixel[x].a = tBuffer[x];
				}
			}
			tPixel += m_TexSize; // 下一行
			tBuffer += tBitmap.pitch; // 下一行
		}
		
		m_CacheTex->AddDirtyRect(&pCache->CacheSize);
	}
	
	return true;
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
	if(!InfoOut)
		return FCYERR_INVAILDPARAM;
	
	//if(Character != L'\t' && iswcntrl(Character))
	//	return FCYERR_INVAILDPARAM;
	//if(Character == L'\t')
	//{
	//	fResult tRet = QueryGlyph(pGraph, L' ', InfoOut);
	//	if(FCYOK(tRet))
	//		InfoOut->Advance.x *= 2.f;
	//	return tRet;
	//}
	
	if(pGraph)
	{
		// 检查是否需要Flush
		//if(m_FreeNodeList == NULL && !m_Dict[Character])
		//	pGraph->Flush();
		
		// 取出文字，绘制到纹理
		FontCacheInfo* pCache = getChar(Character);
		
		InfoOut->Advance = pCache->Advance;
		InfoOut->BrushPos = pCache->BrushPos;
		InfoOut->GlyphPos = pCache->UV;
		InfoOut->GlyphSize = pCache->GlyphSize;
		
		const size_t cidx = (size_t)Character;
		if (m_UsedMark[cidx] == 0) {
			m_UsedMark[cidx] = 1;
			m_UsedCount += 1;
		}
		if (m_UsedCount >= m_Cache.size()) {
			return FCYERR_OUTOFRANGE;
		}
		
		return FCYERR_OK;
	}
	else
	{
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
		
		return FCYERR_OK;
	}
}

fResult f2dFontFileProvider::Flush() {
	m_UsedMark.fill(0);
	m_UsedCount = 0;
	m_CacheTex->Unlock();
	fResult r = m_CacheTex->Upload();
	fuInt tPitch = 0;
	fData tData = NULL;
	if(FCYFAILED(m_CacheTex->Lock(NULL, false, &tPitch, &tData)))
		return FCYERR_INTERNALERR;
	m_CacheTexData = (fcyColor*)tData;
	return r;
}
