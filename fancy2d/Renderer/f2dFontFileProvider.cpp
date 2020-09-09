#include "Renderer/f2dFontFileProvider.h"
#include <fcyException.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

// maxsize 是最大纹理尺寸，因为有些人的电脑甚至加载不出 2048x2048 的纹理
// tcount 是最小字形缓存量
uint32_t calTextureSize(uint32_t width, uint32_t height, uint32_t& xcnt, uint32_t& ycnt, uint32_t maxsize = 1024, uint32_t tcount = 1024) {
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
		2048, // DX9的最大纹理
	};
	const uint32_t tex_size_array_size = sizeof(tex_size) / sizeof(tex_size[0]);
	// 如果指定了更小的纹理尺寸……
	uint32_t _tex_size_array_size = tex_size_array_size;
	for (uint32_t i = 0; i < tex_size_array_size; i += 1) {
		if (tex_size[i] <= maxsize) {
			_tex_size_array_size = i + 1;
		}
		else {
			break;
		}
	}
	// 开始匹配
	const float f_width = (float)(width + 1);
	const float f_height = (float)(height + 1);
	for (uint32_t i = 0; i < _tex_size_array_size; i += 1)
	{
		float f_tex_wh = (float)(tex_size[i] - 1);
		uint32_t x_cnt = (uint32_t)floor(f_tex_wh / f_width);
		uint32_t y_cnt = (uint32_t)floor(f_tex_wh / f_height);
		if ((x_cnt * y_cnt) >= tcount || i >= (_tex_size_array_size - 1)) {
			xcnt = x_cnt;
			ycnt = y_cnt;
			return tex_size[i];
		}
	}
	// 太大了
	xcnt = 1;
	ycnt = 1;
	return tex_size[_tex_size_array_size - 1];
}

// 宽度单位到像素
float widthSizeToPixel(FT_Face Face, int Size)
{
	float tXScale = Face->size->metrics.x_scale / 65536.f;
	return (Size / 64.f) * tXScale;
}

// 高度单位到像素
float heightSizeToPixel(FT_Face Face, int Size)
{
	float tYScale = Face->size->metrics.y_scale / 65536.f;
	return (Size / 64.f) * tYScale;
}

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

////////////////////////////////////////////////////////////////////////////////

// 引擎实现接口

f2dFontFileProvider::f2dFontFileProvider(f2dRenderDevice* pParent,
	f2dStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag)
{
	m_pParent = pParent;
	
	f2dFontProviderParam param;
	param.font_bbox = BBoxSize;
	param.glyph_count = 1024;
	param.texture_size = 2048;
	
	f2dTrueTypeFontParam fonts[1];
	fonts[0].font_file = pStream;
	fonts[0].font_source = nullptr;
	fonts[0].font_face = (fInt)FaceIndex;
	fonts[0].font_size = FontSize;
	
	if (!openFonts(param, fonts, 1)) {
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "open fonts failed.");
	}
	if (!makeCacheMain()) {
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "make glyph cache failed.");
	}
	
	m_pParent->AttachListener(this); // 绑定监听器
}

f2dFontFileProvider::f2dFontFileProvider(f2dRenderDevice* pParent,
	fcyMemStream* pStream, const fcyVec2& FontSize, const fcyVec2& BBoxSize, fuInt FaceIndex, F2DFONTFLAG Flag)
{
	m_pParent = pParent;
	
	f2dFontProviderParam param;
	param.font_bbox = BBoxSize;
	param.glyph_count = 1024;
	param.texture_size = 2048;
	
	f2dTrueTypeFontParam fonts[1];
	fonts[0].font_file = nullptr;
	fonts[0].font_source = pStream;
	fonts[0].font_face = (fInt)FaceIndex;
	fonts[0].font_size = FontSize;
	
	if (!openFonts(param, fonts, 1)) {
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "open fonts failed.");
	}
	if (!makeCacheMain()) {
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "make glyph cache failed.");
	}
	
	m_pParent->AttachListener(this); // 绑定监听器
}

f2dFontFileProvider::f2dFontFileProvider(f2dRenderDevice* pParent,
		f2dFontProviderParam param, f2dTrueTypeFontParam* fonts, fuInt count)
{
	m_pParent = pParent;
	
	if (!openFonts(param, fonts, count)) {
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "open fonts failed.");
	}
	if (!makeCacheMain()) {
		throw fcyException("f2dFontFileProvider::f2dFontFileProvider", "make glyph cache failed.");
	}
	
	m_pParent->AttachListener(this); // 绑定监听器
}

f2dFontFileProvider::~f2dFontFileProvider()
{
	// 释放纹理
	if (m_CacheTex) {
		m_CacheTex->Unlock();
	}
	FCYSAFEKILL(m_CacheTex);
	
	// 关闭FreeType
	closeFonts();

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

// 字体管理

bool f2dFontFileProvider::openFonts(f2dFontProviderParam param, f2dTrueTypeFontParam* fonts, fuInt count) {
	closeFonts();
	if (count == 0) {
		return false; // 你在搞什么飞机
	}
	// 打开 FreeType2 库
	if (FT_Err_Ok != FT_Init_FreeType(&m_FontLib)) {
		return false;
	}
	// 保存参数
	m_BaseParam = param;
	m_Param.resize(count);
	for (fuInt i = 0; i < count; i++) {
		m_Param[i] = fonts[i];
	}
	// 逐个打开字体
	for (auto& ftf : m_Param) {
		// 准备数据
		FontData data;
		memset(&data, 0, sizeof(data));
		// 准备流
		if (ftf.font_file != nullptr) {
			// 如果是抽象流
			ftf.font_file->SetPosition(FCYSEEKORIGIN_BEG, 0);
			data.stream = ftf.font_file;
			// 准备 FT2 字体流
			data.ftargs.flags = FT_OPEN_STREAM;
			data.ftargs.stream = &data.ftstream;
			data.ftstream.size = (unsigned long)ftf.font_file->GetLength();
			data.ftstream.descriptor.pointer = ftf.font_file;
			data.ftstream.read = streamRead;
			data.ftstream.close = streamClose;
		}
		else if (ftf.font_source != nullptr) {
			// 如果是内存流
			ftf.font_source->SetPosition(FCYSEEKORIGIN_BEG, 0); // 估计不需要，因为它直接读内存块
			data.stream = (f2dStream*)ftf.font_source;
			// 准备 FT2 字体流
			data.ftargs.flags = FT_OPEN_MEMORY;
			data.ftargs.memory_base = ftf.font_source->GetInternalBuffer();
			data.ftargs.memory_size = ftf.font_source->GetLength();
		}
		else {
			continue; // 什么？你毛都没给我，怎么打开字体
		}
		data.stream->AddRef();
		// 尝试打开字体
		if (FT_Err_Ok == FT_Open_Face(m_FontLib, &data.ftargs, ftf.font_face, &data.ftFace)) {
			// 设置一些参数
			FT_Set_Pixel_Sizes(data.ftFace, (FT_UInt)ftf.font_size.x, (FT_UInt)ftf.font_size.y);
			// 计算包围盒
			data.ftBBox.x = ceil(widthSizeToPixel(data.ftFace, abs(data.ftFace->bbox.xMax - data.ftFace->bbox.xMin)));
			data.ftBBox.y = ceil(heightSizeToPixel(data.ftFace, abs(data.ftFace->bbox.yMax - data.ftFace->bbox.yMin)));
			// 计算一些额外的度量值
			data.ftLineHeight = heightSizeToPixel(data.ftFace, data.ftFace->height);
			data.ftAscender = heightSizeToPixel(data.ftFace, data.ftFace->ascender);
			data.ftDescender = heightSizeToPixel(data.ftFace, data.ftFace->descender);
			// 现在可以把数据保存下来了
			m_Fonts.emplace_back(std::move(data));
		}
		else {
			// 没成功打开
			data.stream->Release();
		}
	}
	// 至少找到一个fallback字体
	if (m_Fonts.size() > 0) {
		m_Fonts.back().fallback = true;
	}
	// 计算公共参数
	for (auto& f : m_Fonts) {
		m_FontsInfo.ftBBox.x = max(m_FontsInfo.ftBBox.x, f.ftBBox.x);
		m_FontsInfo.ftBBox.y = max(m_FontsInfo.ftBBox.y, f.ftBBox.y);
		m_FontsInfo.ftLineHeight = max(m_FontsInfo.ftLineHeight, f.ftLineHeight);
		m_FontsInfo.ftAscender = max(m_FontsInfo.ftAscender, f.ftAscender);
		m_FontsInfo.ftDescender = min(m_FontsInfo.ftDescender, f.ftDescender);
	}
	return m_Fonts.size() > 0; // 如果一个都没有，啧啧啧……
}

void f2dFontFileProvider::closeFonts() {
	// 关闭 FT2 所有相关的东西
	for (auto& f : m_Fonts) {
		// 先关闭字体
		if (f.ftFace != nullptr) {
			FT_Done_Face(f.ftFace);
			f.ftFace = nullptr;
		}
		// 然后关闭字体流
		if (f.stream != nullptr) {
			f.stream->Release();
			f.stream = nullptr;
		}
	}
	m_Fonts.clear(); // 全部清空
	if (m_FontLib != nullptr) {
		FT_Done_FreeType(m_FontLib); // 关闭 FT2 库
		m_FontLib = nullptr;
	}
	memset(&m_FontsInfo, 0, sizeof(m_FontsInfo)); // 清空公共字体信息
	
	m_Param.clear(); // 清空参数
	// 重置信息
	memset(&m_BaseParam, 0, sizeof(m_BaseParam));
}

// 字形管理

bool f2dFontFileProvider::findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index) {
	for (auto& f : m_Fonts) {
		FT_UInt i = FT_Get_Char_Index(f.ftFace, code);
		if (i != 0 || f.fallback) {
			face = f.ftFace;
			index = i;
			return true;
		}
	}
	return false;
}

f2dGlyphInfo f2dFontFileProvider::getGlyphInfo(fCharW Char) {
	FT_Face face = nullptr;
	FT_UInt index = 0;
	f2dGlyphInfo info;
	if (findGlyph((FT_ULong)Char, face, index)) {
		// 加载文字到字形槽
		FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
		FT_Bitmap* tBitmap = &face->glyph->bitmap;
		
		// 写入对应属性
		info.Advance = fcyVec2(face->glyph->advance.x / 64.f, face->glyph->advance.y / 64.f);
		info.BrushPos = fcyVec2((float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top);
		info.GlyphSize = fcyVec2((float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows);
	}
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

bool f2dFontFileProvider::makeCacheMain() {
	// 产生字体缓存
	auto caldata = [&](uint32_t maxsize)->void {
		auto& bbox = m_BaseParam.font_bbox;
		// 计算最大字形大小
		if (bbox.x >= 1.0f && bbox.y >= 1.0f) {
			// 这个参数平时没卵用，专门用来杀了思源系列字体的妈，没事搞超大字形包围盒，导致字形缓存TMD空间压根不够用
			m_MaxGlyphWidth = (fuInt)ceil(bbox.x);
			m_MaxGlyphHeight = (fuInt)ceil(bbox.y);
		}
		else {
			m_MaxGlyphWidth = (fuInt)m_FontsInfo.ftBBox.x;
			m_MaxGlyphHeight = (fuInt)m_FontsInfo.ftBBox.y;
		}
		// 计算所需的纹理大小
		m_TexSize = calTextureSize(m_MaxGlyphWidth, m_MaxGlyphHeight, m_CacheXCount, m_CacheYCount, maxsize, m_BaseParam.glyph_count);
		// 处理一些异常情况（比如字形比纹理大）
		m_MaxGlyphWidth = (m_MaxGlyphWidth > m_TexSize) ? m_TexSize : m_MaxGlyphWidth;
		m_MaxGlyphHeight = (m_MaxGlyphHeight > m_TexSize) ? m_TexSize : m_MaxGlyphHeight;
	};
	caldata(min(2048, m_BaseParam.texture_size));
	if(!makeCache(m_TexSize))
	{
		caldata(min(1024, m_BaseParam.texture_size)); // 改小一点重新试一次
		// 这次失败就真没救了
		if(!makeCache(m_TexSize))
		{
			// 关闭FreeType
			closeFonts();
			return false;
		}
	}
	return true;
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
	FT_Face face = nullptr;
	FT_UInt index = 0;
	if (findGlyph((FT_ULong)Char, face, index)) {
		// 加载文字到字形槽并渲染
		FT_Load_Glyph(face, index, FT_LOAD_RENDER);
		FT_Bitmap& tBitmap = face->glyph->bitmap;
		
		// 写入对应属性
		pCache->Character = Char;
		pCache->Advance = fcyVec2(face->glyph->advance.x / 64.f, face->glyph->advance.y / 64.f);
		pCache->BrushPos = fcyVec2((float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top);
		pCache->GlyphSize = fcyVec2((float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows); 
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
	return false;
}

// 接口

fFloat f2dFontFileProvider::GetLineHeight()
{
	return m_FontsInfo.ftLineHeight;
}

fFloat f2dFontFileProvider::GetAscender()
{
	return m_FontsInfo.ftAscender;
}

fFloat f2dFontFileProvider::GetDescender()
{
	return m_FontsInfo.ftDescender;
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
			return FCYERR_OUTOFRANGE; // 字形缓存槽满了，需要Flush一次
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
