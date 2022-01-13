#include "Renderer/f2dTrueTypeFontProvider.h"
#include <stdexcept>
#include <algorithm>
#include "Common/utf.hpp"

struct SharedFreeTypeLibrary
{
	FT_Library lib;
	void lazyLoad()
	{
		if (!lib)
		{
			if (FT_Err_Ok != FT_Init_FreeType(&lib))
			{
				lib = NULL;
				throw std::runtime_error("FT_Init_FreeType failed.");
			}
		}
	}
	SharedFreeTypeLibrary() : lib(NULL)
	{
	}
	~SharedFreeTypeLibrary()
	{
		if (lib)
		{
			FT_Done_FreeType(lib);
			lib = NULL;
		}
	}
	static SharedFreeTypeLibrary& get()
	{
		static SharedFreeTypeLibrary v;
		v.lazyLoad();
		return v;
	}
};

#define G_FT_Library (SharedFreeTypeLibrary::get().lib)

static unsigned long ft_stream_read(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
	f2dStream* pStream = (f2dStream*)stream->descriptor.pointer;

	pStream->Lock();

	fLen tRealReaded = 0;
	pStream->SetPosition(FCYSEEKORIGIN_BEG, offset);
	pStream->ReadBytes(buffer, count, &tRealReaded);

	pStream->Unlock();

	return (unsigned long)tRealReaded;
}
static void ft_stream_close(FT_Stream stream)
{
	// 交给类析构函数处理
}

// 宽度单位到像素
static float widthSizeToPixel(FT_Face Face, int Size)
{
	float tXScale = Face->size->metrics.x_scale / 65536.f;
	return (Size / 64.f) * tXScale;
}
// 高度单位到像素
static float heightSizeToPixel(FT_Face Face, int Size)
{
	float tYScale = Face->size->metrics.y_scale / 65536.f;
	return (Size / 64.f) * tYScale;
}

f2dTrueTypeFontProvider::Image2D::Image2D()
{
	std::memset(data, 0, sizeof(data));
}

void f2dTrueTypeFontProvider::closeFonts()
{
	for (auto& f : m_Font)
	{
		// 先关闭字体
		if (f.ftFace)
		{
			FT_Done_Face(f.ftFace);
			f.ftFace = NULL;
		}
		// 然后关闭字体文件流
		if (f.stream)
		{
			f.stream->Release();
			f.stream = nullptr;
		}
	}
	m_Font.clear();
}
bool f2dTrueTypeFontProvider::openFonts(f2dTrueTypeFontParam* fonts, fuInt count)
{
	closeFonts();
	if (count == 0)
	{
		return false; // 你在搞什么飞机
	}
	
	// 逐个打开字体
	m_Font.resize(count);
	for (fuInt i = 0; i < count; i++)
	{
		// 准备数据
		FreeTypeFontData& data = m_Font[i];
		std::memset(&data, 0, sizeof(data));
		// 准备流
		if (fonts[i].font_file)
		{
			// 如果是非内存流
			fonts[i].font_file->SetPosition(FCYSEEKORIGIN_BEG, 0);
			data.stream = fonts[i].font_file;
			// 准备 FT2 字体流
			data.ftstream.size = (unsigned long)data.stream->GetLength();
			data.ftstream.descriptor.pointer = data.stream;
			data.ftstream.read = &ft_stream_read;
			data.ftstream.close = &ft_stream_close;
			data.ftargs.flags = FT_OPEN_STREAM;
			data.ftargs.stream = &data.ftstream;
		}
		else if (fonts[i].font_source)
		{
			// 如果是内存流
			data.stream = (f2dStream*)fonts[i].font_source;
			// 准备 FT2 字体流
			data.ftargs.flags = FT_OPEN_MEMORY;
			data.ftargs.memory_base = (FT_Byte*)fonts[i].font_source->GetInternalBuffer();
			data.ftargs.memory_size = (FT_Long)fonts[i].font_source->GetLength();
		}
		else
		{
			continue; // 什么？你毛都没给我，怎么打开字体
		}
		data.stream->AddRef();
		// 尝试打开字体
		if (FT_Err_Ok == FT_Open_Face(G_FT_Library, &data.ftargs, fonts[i].font_face, &data.ftFace))
		{
			// 设置一些参数
			FT_Set_Pixel_Sizes(data.ftFace, (FT_UInt)fonts[i].font_size.x, (FT_UInt)fonts[i].font_size.y);
			// 计算一些额外的度量值
			data.ftLineHeight = heightSizeToPixel(data.ftFace, data.ftFace->height);
			data.ftAscender = heightSizeToPixel(data.ftFace, data.ftFace->ascender);
			data.ftDescender = heightSizeToPixel(data.ftFace, data.ftFace->descender);
		}
		else
		{
			// 没成功打开
			data.ftFace = NULL;
			data.stream->Release();
		}
	}
	
	// 检查结果
	if (m_Font.size() > 0)
	{
		// 至少找到一个fallback字体
		m_Font.back().bFallback = true;
		// 计算公共参数
		for (auto& f : m_Font)
		{
			m_CommonInfo.ftLineHeight = std::max(m_CommonInfo.ftLineHeight, f.ftLineHeight);
			m_CommonInfo.ftAscender = std::max(m_CommonInfo.ftAscender, f.ftAscender);
			m_CommonInfo.ftDescender = std::min(m_CommonInfo.ftDescender, f.ftDescender);
		}
		return true;
	}
	else
	{
		closeFonts();
		return false;
	}
}
bool f2dTrueTypeFontProvider::addTexture()
{
	m_Tex.emplace_back();
	auto& t = m_Tex.back();
	if (FCYFAILED(m_pParent->CreateDynamicTexture(t.image.width, t.image.height, &t.texture)))
	{
		t.texture = nullptr;
		return false;
	}
	return true;
}
bool f2dTrueTypeFontProvider::findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index)
{
	for (auto& f : m_Font)
	{
		if (f.ftFace)
		{
			FT_UInt i = FT_Get_Char_Index(f.ftFace, code);
			if (i != 0 || f.bFallback)
			{
				face = f.ftFace;
				index = i;
				return true;
			}
		}
	}
	return false;
}
bool f2dTrueTypeFontProvider::writeBitmapToCache(GlyphCacheInfo& info, FT_Bitmap& bitmap)
{
	// 太大的不要，滚
	if (bitmap.width > (TEXTURE_SIZE - 2) || bitmap.rows > (TEXTURE_SIZE - 2))
	{
		return false;
	}
	// 搞到一个位置
	GlyphCache2D* pt = nullptr;
	do
	{
		for (auto& t : m_Tex)
		{
			// 初始化，留出 1 像素左上边缘
			if (t.pen_x == 0 || t.pen_y == 0)
			{
				t.pen_x = 1;
				t.pen_y = 1;
			}
			// 这行能塞下吗，留出 1 像素右下边缘
			if ((t.pen_x + bitmap.width) < (t.image.width - 1) && (t.pen_y + bitmap.rows) < (t.image.height - 1))
			{
				pt = &t;
				break;
			}
			// 换行，留出 1 像素行边缘
			fuInt const new_pen_x = 1;
			fuInt const new_pen_y = std::max(t.pen_y, t.pen_bottom + 1);
			// 这行能塞下吗，留出 1 像素右下边缘
			if ((new_pen_x + bitmap.width) < (t.image.width - 1) && (new_pen_y + bitmap.rows) < (t.image.height - 1))
			{
				t.pen_x = new_pen_x;
				t.pen_y = new_pen_y;
				pt = &t;
				break;
			}
		}
		if (pt)
		{
			break;
		}
		else
		{
			// 该添新丁了
			if (!addTexture())
			{
				return false;
			}
		}
	} while (!pt);
	GlyphCache2D& t = *pt;
	// 写入字形数据
	info.TextureIndex = pt - m_Tex.data();
	info.GlyphPos.a.x = (float)t.pen_x / (float)t.image.width;
	info.GlyphPos.a.y = (float)t.pen_y / (float)t.image.height;
	info.GlyphPos.b.x = (float)(t.pen_x + bitmap.width) / (float)t.image.width;
	info.GlyphPos.b.y = (float)(t.pen_y + bitmap.rows) / (float)t.image.height;
	// 写入 bitmap 数据，同时写入 1 像素宽的透明边缘，写的有点乱，主要是为了最大化减少 CPU Cache Miss
	for (int x = 0; x < (bitmap.width + 2); x += 1) // 上 1 像素宽的边，宽度是 bitmap 的宽度再加 2 像素
	{
		t.image.pixel(t.pen_x - 1 + x, t.pen_y - 1) = fcyColor(0x00000000);
	}
	for (int y = 0; y < bitmap.rows; y += 1)
	{
		t.image.pixel(t.pen_x - 1, t.pen_y + y) = fcyColor(0x00000000); // 左 1 像素宽的边
		for (int x = 0; x < bitmap.width; x += 1)
		{
			auto& c = t.image.pixel(t.pen_x + x, t.pen_y + y);
			c.r = 255;
			c.g = 255;
			c.b = 255;
			c.a = bitmap.buffer[y * bitmap.pitch + x];
		}
		t.image.pixel(t.pen_x + bitmap.width, t.pen_y + y) = fcyColor(0x00000000); // 右 1 像素宽的边
	}
	for (int x = 0; x < (bitmap.width + 2); x += 1) // 下 1 像素宽的边，宽度是 bitmap 的宽度再加 2 像素
	{
		t.image.pixel(t.pen_x - 1 + x, t.pen_y + bitmap.rows) = fcyColor(0x00000000);
	}
	// 更新脏区域
	if (t.dirty_l == INVALID_RECT)
	{
		t.dirty_l = t.pen_x - 1;
		t.dirty_t = t.pen_y - 1;
		t.dirty_r = t.pen_x + bitmap.width + 1;
		t.dirty_b = t.pen_y + bitmap.rows + 1;
	}
	else
	{
		t.dirty_l = std::min(t.dirty_l, t.pen_x - 1);
		t.dirty_t = std::min(t.dirty_t, t.pen_y - 1);
		t.dirty_r = std::max(t.dirty_r, t.pen_x + bitmap.width + 1);
		t.dirty_b = std::max(t.dirty_b, t.pen_y + bitmap.rows + 1);
	}
	// 更新缓存
	t.pen_x += bitmap.width + 1;
	t.pen_bottom = std::max(t.pen_bottom, t.pen_y + bitmap.rows);
	return true;
}
bool f2dTrueTypeFontProvider::renderCache(fCharU Char)
{
	FT_Face face = nullptr;
	FT_UInt index = 0;
	if (findGlyph((FT_ULong)Char, face, index))
	{
		// 加载文字到字形槽并渲染
		FT_Load_Glyph(face, index, FT_LOAD_RENDER);
		FT_GlyphSlot& glyph = face->glyph;
		FT_Bitmap& bitmap = glyph->bitmap;
		// 写入对应属性
		GlyphCacheInfo cache = {};
		cache.Character = Char;
		cache.Advance = fcyVec2((float)glyph->advance.x / 64.f, (float)glyph->advance.y / 64.f);
		cache.BrushPos = fcyVec2((float)glyph->bitmap_left, (float)glyph->bitmap_top);
		cache.GlyphSize = fcyVec2((float)glyph->bitmap.width, (float)glyph->bitmap.rows);
		if (!writeBitmapToCache(cache, bitmap))
		{
			return false;
		}
		// 塞表里
		m_Map.emplace(Char, cache);
	}
	return false;
}
f2dTrueTypeFontProvider::GlyphCacheInfo* f2dTrueTypeFontProvider::getGlyphCacheInfo(fCharU Char)
{
	auto it = m_Map.find(Char);
	if (it != m_Map.end())
	{
		return &it->second;
	}
	else if (renderCache(Char))
	{
		return &m_Map[Char];
	}
	else
	{
		return nullptr;
	}
}

fFloat f2dTrueTypeFontProvider::GetLineHeight()
{
	return m_CommonInfo.ftLineHeight;
}
fFloat f2dTrueTypeFontProvider::GetAscender()
{
	return m_CommonInfo.ftAscender;
}
fFloat f2dTrueTypeFontProvider::GetDescender()
{
	return m_CommonInfo.ftDescender;
}
fuInt f2dTrueTypeFontProvider::GetCacheTextureCount()
{
	return (fuInt)m_Tex.size();
}
f2dTexture2D* f2dTrueTypeFontProvider::GetCacheTexture(fuInt index)
{
	if (index < m_Tex.size())
	{
		return m_Tex[index].texture;
	}
	return nullptr;
}
fInt f2dTrueTypeFontProvider::GetCacheCount()
{
	return (fInt)m_Map.size();
}
fInt f2dTrueTypeFontProvider::GetCacheTexSize()
{
	return TEXTURE_SIZE;
}

fResult f2dTrueTypeFontProvider::CacheString(fcStrW String)
{
	fuInt tLen = wcslen(String);
	for (fuInt i = 0; i < tLen; ++i)
	{
		getGlyphCacheInfo((fCharU)String[i]);
	}
	return FCYERR_OK;
}
fResult f2dTrueTypeFontProvider::CacheStringU8(fcStr Text, fuInt Count)
{
	// utf-8 迭代器
	char32_t code_ = 0;
	utf::utf8reader reader_(Text, Count);
	if (Count == static_cast<fuInt>(-1))
	{
		reader_ = utf::utf8reader(Text); // 没给长度的情况
	}
	while (reader_(code_))
	{
		getGlyphCacheInfo(code_);
	}
	return FCYERR_OK;
}
fResult f2dTrueTypeFontProvider::QueryGlyph(f2dGraphics* pGraph, fCharU Character, f2dGlyphInfo* InfoOut)
{
	if (!InfoOut)
	{
		return FCYERR_INVAILDPARAM;
	}
	if (GlyphCacheInfo* info = getGlyphCacheInfo(Character))
	{
		InfoOut->TextureIndex = info->TextureIndex;
		InfoOut->GlyphPos = info->GlyphPos;
		InfoOut->GlyphSize = info->GlyphSize;
		InfoOut->BrushPos = info->BrushPos;
		InfoOut->Advance = info->Advance;
		if (pGraph)
		{
			return Flush();
		}
		return FCYERR_OK;
	}
	return FCYERR_INTERNALERR;
}
fResult f2dTrueTypeFontProvider::Flush()
{
	for (auto& t : m_Tex)
	{
		if (t.dirty_l != 0x7FFFFFFF)
		{
			fcyRect rc((fFloat)t.dirty_l, (fFloat)t.dirty_t, (fFloat)t.dirty_r, (fFloat)t.dirty_b);
			if (FCYFAILED(t.texture->Update(&rc, (fData)&t.image.pixel(t.dirty_l, t.dirty_t), t.image.pitch)))
			{
				return FCYERR_INTERNALERR;
			}
			t.dirty_l = 0x7FFFFFFF;
			t.dirty_t = 0x7FFFFFFF;
			t.dirty_r = 0x7FFFFFFF;
			t.dirty_b = 0x7FFFFFFF;
		}
	}
	return FCYERR_OK;
}

void f2dTrueTypeFontProvider::OnRenderDeviceLost()
{
	// 全标记为脏
	for (auto& t : m_Tex)
	{
		t.dirty_l = 0;
		t.dirty_t = 0;
		t.dirty_r = t.image.width;
		t.dirty_b = t.pen_bottom + 1;
	}
}
void f2dTrueTypeFontProvider::OnRenderDeviceReset()
{
	// 等一个幸运儿调用 Flush
}

f2dTrueTypeFontProvider::f2dTrueTypeFontProvider(f2dRenderDevice* pParent, f2dTrueTypeFontParam* fonts, fuInt count)
	: m_pParent(pParent)
{
	if (!openFonts(fonts, count))
	{
		throw fcyException("f2dTrueTypeFontProvider::f2dTrueTypeFontProvider", "open fonts failed.");
	}
	if (!addTexture())
	{
		throw fcyException("f2dTrueTypeFontProvider::f2dTrueTypeFontProvider", "create f2dTexture2D failed.");
	}
	m_pParent->AttachListener(this);
}
f2dTrueTypeFontProvider::~f2dTrueTypeFontProvider()
{
	m_pParent->RemoveListener(this);
	closeFonts();
	for (auto& t : m_Tex)
	{
		if (t.texture)
		{
			t.texture->Release();
			t.texture = nullptr;
		}
	}
}
