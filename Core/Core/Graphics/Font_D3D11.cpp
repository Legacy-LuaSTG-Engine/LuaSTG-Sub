#include "Core/Graphics/Font_D3D11.hpp"
#include "Core/FileManager.hpp"
#include "utility/utf.hpp"
#include "utility/encoding.hpp"

static bool findSystemFont(std::string_view name, std::string& u8_path);

namespace Core::Graphics
{
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

	// 宽度单位到像素
	inline float widthSizeToPixel(FT_Face Face, int Size)
	{
		float tXScale = Face->size->metrics.x_scale / 65536.f;
		return (Size / 64.f) * tXScale;
	}
	// 高度单位到像素
	inline float heightSizeToPixel(FT_Face Face, int Size)
	{
		float tYScale = Face->size->metrics.y_scale / 65536.f;
		return (Size / 64.f) * tYScale;
	}

	Image2D::Image2D()
	{
		std::memset(data, 0, sizeof(data));
	}

	void TrueTypeGlyphManager_D3D11::onDeviceCreate()
	{
		// 等一个幸运儿调用 flush
	}
	void TrueTypeGlyphManager_D3D11::onDeviceDestroy()
	{
		// 全标记为脏
		for (auto& t : m_tex)
		{
			t.dirty_l = 0;
			t.dirty_t = 0;
			t.dirty_r = t.image.width;
			t.dirty_b = t.pen_bottom + 1; // 多一个像素的边缘
		}
	}

	void TrueTypeGlyphManager_D3D11::closeFonts()
	{
		// 先关闭字体
		for (auto& f : m_font)
		{
			if (f.ft_face)
			{
				FT_Done_Face(f.ft_face);
				f.ft_face = NULL;
			}
		}
		m_font.clear();
	}
	bool TrueTypeGlyphManager_D3D11::openFonts(TrueTypeFontInfo* fonts, size_t count)
	{
		closeFonts();
		if (!fonts || count == 0)
		{
			assert(false); return false;
		}

		// 逐个打开字体
		m_font.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			// 准备数据
			FreeTypeFontData& data = m_font[i];
			auto setupFontFace = [&]()
			{
				// 设置一些参数
				if (FT_Err_Ok != FT_Set_Pixel_Sizes(data.ft_face, (FT_UInt)fonts[i].font_size.x, (FT_UInt)fonts[i].font_size.y))
				{
					FT_Done_Face(data.ft_face);
					data.ft_face = NULL;
					return;
				}
				// 计算一些度量值
				data.ft_line_height = heightSizeToPixel(data.ft_face, data.ft_face->height);
				data.ft_ascender = heightSizeToPixel(data.ft_face, data.ft_face->ascender);
				data.ft_descender = heightSizeToPixel(data.ft_face, data.ft_face->descender);
			};
			auto openFromFile = [&](std::string_view path)
			{
				// 打开
				if (FT_Err_Ok != FT_New_Face(G_FT_Library, path.data(), (FT_Long)fonts[i].font_face, &data.ft_face))
				{
					return;
				}
				setupFontFace();
			};
			auto openFromBuffer = [&]()
			{
				// 打开
				if (FT_Err_Ok != FT_New_Memory_Face(G_FT_Library, (FT_Byte*)data.buffer.data(), (FT_Long)data.buffer.size(), (FT_Long)fonts[i].font_face, &data.ft_face))
				{
					return;
				}
				setupFontFace();
			};
			if (!fonts[i].is_buffer)
			{
				// 先看看是不是要从系统加载
				std::string u8_path(fonts[i].source);
				if (!GFileManager().containEx(u8_path))
				{
					if (!findSystemFont(fonts[i].source, u8_path))
					{
						continue; // 还是找不到
					}
				}
				// 如果是路径，尝试从文件打开
				if (!fonts[i].is_force_to_file)
				{
					// 读取进内存
					if (GFileManager().loadEx(u8_path, data.buffer))
					{
						openFromBuffer();
					}
				}
				else
				{
					// 从文件打开
					openFromFile(u8_path);
				}
			}
			else if (fonts[i].source.data() && fonts[i].source.size() > 0)
			{
				// 如果是一块二进制数据，复制下来备用
				data.buffer.resize(fonts[i].source.size());
				std::memcpy(data.buffer.data(), fonts[i].source.data(), fonts[i].source.size());
				openFromBuffer();
			}
			else
			{
				assert(false); continue; // 您数据呢？
			}
		}

		// 检查结果
		if (m_font.size() > 0)
		{
			// 至少找到一个fallback字体
			m_font.back().is_fallback = true;
			// 计算公共参数
			for (auto& f : m_font)
			{
				m_common_info.ft_line_height = std::max(m_common_info.ft_line_height, f.ft_line_height);
				m_common_info.ft_ascender = std::max(m_common_info.ft_ascender, f.ft_ascender);
				m_common_info.ft_descender = std::min(m_common_info.ft_descender, f.ft_descender);
			}
			return true;
		}
		else
		{
			closeFonts();
			return false;
		}
	}
	bool TrueTypeGlyphManager_D3D11::addTexture()
	{
		m_tex.emplace_back();
		auto& t = m_tex.back();
		if (!m_device->createTexture(Vector2U(t.image.width, t.image.height), ~t.texture))
		{
			return false;
		}
		return true;
	}
	bool TrueTypeGlyphManager_D3D11::findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index)
	{
		for (auto& f : m_font)
		{
			if (f.ft_face)
			{
				FT_UInt const i = FT_Get_Char_Index(f.ft_face, code);
				if (i != 0 || f.is_fallback)
				{
					face = f.ft_face;
					index = i;
					return true;
				}
			}
		}
		return false;
	}
	bool TrueTypeGlyphManager_D3D11::writeBitmapToCache(GlyphCacheInfo& info, FT_Bitmap& bitmap)
	{
		// 太大的不要，滚
		if (bitmap.width > (TEXTURE_SIZE - 2) || bitmap.rows > (TEXTURE_SIZE - 2))
		{
			assert(false); return false;
		}
		// 搞到一个位置
		GlyphCache2D* pt = nullptr;
		do
		{
			for (auto& t : m_tex)
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
				uint32_t const new_pen_x = 1;
				uint32_t const new_pen_y = std::max(t.pen_y, t.pen_bottom + 1);
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
		info.texture_index = (uint32_t)(pt - m_tex.data());
		info.texture_rect.a.x = (float)t.pen_x / (float)t.image.width;
		info.texture_rect.a.y = (float)t.pen_y / (float)t.image.height;
		info.texture_rect.b.x = (float)(t.pen_x + bitmap.width) / (float)t.image.width;
		info.texture_rect.b.y = (float)(t.pen_y + bitmap.rows) / (float)t.image.height;
		// 写入 bitmap 数据，同时写入 1 像素宽的透明边缘，写的有点乱，主要是为了最大化减少 CPU Cache Miss
		for (int x = 0; x < (int)(bitmap.width + 2); x += 1) // 上 1 像素宽的边，宽度是 bitmap 的宽度再加 2 像素
		{
			t.image.pixel(t.pen_x - 1 + x, t.pen_y - 1) = Color4B(0x00FFFFFF);
		}
		for (int y = 0; y < (int)bitmap.rows; y += 1)
		{
			t.image.pixel(t.pen_x - 1, t.pen_y + y) = Color4B(0x00FFFFFF); // 左 1 像素宽的边
			for (int x = 0; x < (int)bitmap.width; x += 1)
			{
				uint32_t const alpha = (uint32_t)bitmap.buffer[y * bitmap.pitch + x] << 24;
				t.image.pixel(t.pen_x + x, t.pen_y + y) = Color4B(alpha | 0x00FFFFFF);
			}
			t.image.pixel(t.pen_x + bitmap.width, t.pen_y + y) = Color4B(0x00FFFFFF); // 右 1 像素宽的边
		}
		for (int x = 0; x < (int)(bitmap.width + 2); x += 1) // 下 1 像素宽的边，宽度是 bitmap 的宽度再加 2 像素
		{
			t.image.pixel(t.pen_x - 1 + x, t.pen_y + bitmap.rows) = Color4B(0x00FFFFFF);
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
	GlyphCacheInfo* TrueTypeGlyphManager_D3D11::getGlyphCacheInfo(uint32_t codepoint)
	{
		auto it = m_map.find(codepoint);
		if (it != m_map.end())
		{
			return &it->second;
		}
		else if (renderCache(codepoint))
		{
			return &m_map[codepoint];
		}
		else
		{
			return nullptr;
		}
	}
	bool TrueTypeGlyphManager_D3D11::renderCache(uint32_t codepoint)
	{
		FT_Face face = nullptr;
		FT_UInt index = 0;
		if (findGlyph((FT_ULong)codepoint, face, index))
		{
			// 加载文字到字形槽并渲染
			FT_Load_Glyph(face, index, FT_LOAD_RENDER);
			FT_GlyphSlot& glyph = face->glyph;
			FT_Bitmap& bitmap = glyph->bitmap;
			// 写入对应属性
			GlyphCacheInfo cache = {};
			cache.size = Vector2F((float)glyph->bitmap.width, (float)glyph->bitmap.rows);
			cache.position = Vector2F((float)glyph->bitmap_left, (float)glyph->bitmap_top);
			cache.advance = Vector2F((float)glyph->advance.x / 64.f, (float)glyph->advance.y / 64.f);
			cache.codepoint = codepoint;
			if (!writeBitmapToCache(cache, bitmap))
			{
				return false;
			}
			// 塞表里
			m_map.emplace(codepoint, cache);
		}
		return false;
	}

	float TrueTypeGlyphManager_D3D11::getLineHeight()
	{
		return m_common_info.ft_line_height;
	}
	float TrueTypeGlyphManager_D3D11::getAscender()
	{
		return m_common_info.ft_ascender;
	}
	float TrueTypeGlyphManager_D3D11::getDescender()
	{
		return m_common_info.ft_descender;
	}

	uint32_t TrueTypeGlyphManager_D3D11::getTextureCount()
	{
		return (uint32_t)m_tex.size();
	}
	ITexture2D* TrueTypeGlyphManager_D3D11::getTexture(uint32_t index)
	{
		if (index < m_tex.size())
		{
			return m_tex[index].texture.get();
		}
		return nullptr;
	}

	bool TrueTypeGlyphManager_D3D11::cacheGlyph(uint32_t codepoint)
	{
		if (!getGlyphCacheInfo(codepoint))
			return false;
		return true;
	}
	bool TrueTypeGlyphManager_D3D11::cacheString(StringView str)
	{
		// utf-8 迭代器
		char32_t code_ = 0;
		utf::utf8reader reader_(str.data(), str.size());
		bool result_ = true;
		while (reader_(code_))
		{
			if (!getGlyphCacheInfo(code_))
				if (code_ != U'\n')
					result_ = false;
		}
		return result_;
	}
	bool TrueTypeGlyphManager_D3D11::flush()
	{
		for (auto& t : m_tex)
		{
			if (t.dirty_l != INVALID_RECT)
			{
				if (!t.texture->uploadPixelData(
					RectU(t.dirty_l, t.dirty_t, t.dirty_r, t.dirty_b),
					&t.image.pixel(t.dirty_l, t.dirty_t),
					t.image.pitch))
				{
					return false;
				}
				t.dirty_l = INVALID_RECT;
				t.dirty_t = INVALID_RECT;
				t.dirty_r = INVALID_RECT;
				t.dirty_b = INVALID_RECT;
			}
		}
		return true;
	}

	bool TrueTypeGlyphManager_D3D11::getGlyph(uint32_t codepoint, GlyphInfo* p_ref_info, bool no_render)
	{
		if (!p_ref_info)
		{
			assert(false); return false;
		}
		if (GlyphCacheInfo* info = getGlyphCacheInfo(codepoint))
		{
			p_ref_info->texture_index = info->texture_index;
			p_ref_info->texture_rect = info->texture_rect;
			p_ref_info->size = info->size;
			p_ref_info->position = info->position;
			p_ref_info->advance = info->advance;
			return no_render ? true : flush();
		}
		return false;
	}

	TrueTypeGlyphManager_D3D11::TrueTypeGlyphManager_D3D11(IDevice* p_device, TrueTypeFontInfo* p_arr_info, size_t info_count)
		: m_device(p_device)
	{
		if (!openFonts(p_arr_info, info_count))
		{
			throw std::runtime_error("TrueTypeGlyphManager_D3D11::TrueTypeGlyphManager_D3D11 (openFonts)");
		}
		if (!addTexture())
		{
			throw std::runtime_error("TrueTypeGlyphManager_D3D11::TrueTypeGlyphManager_D3D11 (addTexture)");
		}
		cacheGlyph(uint32_t(U' ')); // 先缓存一个空格
		m_device->addEventListener(this);
	}
	TrueTypeGlyphManager_D3D11::~TrueTypeGlyphManager_D3D11()
	{
		m_device->removeEventListener(this);
		closeFonts();
	}

	bool IGlyphManager::create(IDevice* p_device, TrueTypeFontInfo* p_arr_info, size_t info_count, IGlyphManager** pp_glyphmgr)
	{
		try
		{
			*pp_glyphmgr = new TrueTypeGlyphManager_D3D11(p_device, p_arr_info, info_count);
			return true;
		}
		catch (...)
		{
			*pp_glyphmgr = nullptr;
			return false;
		}
	}
}

namespace Core::Graphics
{
	bool TextRenderer_D3D11::drawGlyph(GlyphInfo const& glyph_info, Vector2F const& start_pos)
	{
		// 准备顶点
		IRenderer::DrawVertex vert[4] = {
			IRenderer::DrawVertex(0.0f, 0.0f, m_z, 0.0f, 0.0f, m_color.color()),
			IRenderer::DrawVertex(0.0f, 0.0f, m_z, 0.0f, 0.0f, m_color.color()),
			IRenderer::DrawVertex(0.0f, 0.0f, m_z, 0.0f, 0.0f, m_color.color()),
			IRenderer::DrawVertex(0.0f, 0.0f, m_z, 0.0f, 0.0f, m_color.color()),
		};

		// 计算位置矩形
		vert[0].x = start_pos.x + glyph_info.position.x;
		vert[0].y = start_pos.y + glyph_info.position.y;
		vert[1].x = vert[0].x + glyph_info.size.x;
		vert[1].y = vert[0].y;
		vert[2].x = vert[0].x + glyph_info.size.x;
		vert[2].y = vert[0].y - glyph_info.size.y;
		vert[3].x = vert[0].x;
		vert[3].y = vert[2].y;

		// 复制 UV 坐标
		vert[0].u = glyph_info.texture_rect.a.x;
		vert[0].v = glyph_info.texture_rect.a.y;
		vert[1].u = glyph_info.texture_rect.b.x;
		vert[1].v = glyph_info.texture_rect.a.y;
		vert[2].u = glyph_info.texture_rect.b.x;
		vert[2].v = glyph_info.texture_rect.b.y;
		vert[3].u = glyph_info.texture_rect.a.x;
		vert[3].v = glyph_info.texture_rect.b.y;

		// 获得纹理
		ITexture2D* p_texture = m_glyphmgr->getTexture(glyph_info.texture_index);
		if (!p_texture)
		{
			assert(false); return false;
		}

		// d3d9特有问题，uv坐标要偏移0.5
		if constexpr (false)
		{
			float const u_offset_ = 0.5f / (float)p_texture->getSize().x;
			float const v_offset_ = 0.5f / (float)p_texture->getSize().y;
			vert[0].u += u_offset_;
			vert[0].v += v_offset_;
			vert[1].u += u_offset_;
			vert[1].v += v_offset_;
			vert[2].u += u_offset_;
			vert[2].v += v_offset_;
			vert[3].u += u_offset_;
			vert[3].v += v_offset_;
		}

		// 绘图
		m_renderer->setTexture(p_texture);
		m_renderer->drawQuad(vert);

		return true;
	}
	bool TextRenderer_D3D11::drawGlyphInSpace(GlyphInfo const& glyph_info, Vector3F const& start_pos, Vector3F const& right_vec, Vector3F const& down_vec)
	{
		// 准备顶点
		IRenderer::DrawVertex vert[4] = {
			IRenderer::DrawVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_color.color()),
			IRenderer::DrawVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_color.color()),
			IRenderer::DrawVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_color.color()),
			IRenderer::DrawVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_color.color()),
		};

		// 计算位置矩形
		Vector3F const position = start_pos + (right_vec * glyph_info.position.x) - (down_vec * glyph_info.position.y); // 这里要变成 up_vec
		Vector3F const right_top = position + (right_vec * glyph_info.size.x); // 向右
		Vector3F const right_bottom = position + (right_vec * glyph_info.size.x) + (down_vec * glyph_info.size.y); // 字形右下角顶点位置
		Vector3F const left_bottom = position + (down_vec * glyph_info.size.y); // 向下
		vert[0].x = position.x;
		vert[0].y = position.y;
		vert[0].z = position.z;
		vert[1].x = right_top.x;
		vert[1].y = right_top.y;
		vert[1].z = right_top.z;
		vert[2].x = right_bottom.x;
		vert[2].y = right_bottom.y;
		vert[2].z = right_bottom.z;
		vert[3].x = left_bottom.x;
		vert[3].y = left_bottom.y;
		vert[3].z = left_bottom.z;

		// 复制 UV 坐标
		vert[0].u = glyph_info.texture_rect.a.x;
		vert[0].v = glyph_info.texture_rect.a.y;
		vert[1].u = glyph_info.texture_rect.b.x;
		vert[1].v = glyph_info.texture_rect.a.y;
		vert[2].u = glyph_info.texture_rect.b.x;
		vert[2].v = glyph_info.texture_rect.b.y;
		vert[3].u = glyph_info.texture_rect.a.x;
		vert[3].v = glyph_info.texture_rect.b.y;

		// 获得纹理
		ITexture2D* p_texture = m_glyphmgr->getTexture(glyph_info.texture_index);
		if (!p_texture)
		{
			assert(false); return false;
		}

		// 绘图
		m_renderer->setTexture(p_texture);
		m_renderer->drawQuad(vert);

		return true;
	}

	RectF TextRenderer_D3D11::getTextBoundary(StringView str)
	{
		if (!m_glyphmgr)
		{
			return RectF();
		}

		utf::utf8reader reader_(str.data(), str.size());
		
		GlyphInfo glyph_info = {};
		Vector2F start_pos;
		RectF rect(
			Vector2F(FLT_MAX, -FLT_MAX),
			Vector2F(-FLT_MAX, FLT_MAX));
		bool is_updated = false;
		float const line_height = m_glyphmgr->getLineHeight();

		char32_t code_ = 0;
		while (reader_(code_))
		{
			if (code_ == U'\n')
			{
				start_pos.x = 0.0f;
				start_pos.y -= line_height;
				continue;
			}
			if (m_glyphmgr->getGlyph(code_, &glyph_info, true))
			{
				// 更新包围盒
				float const left = start_pos.x + glyph_info.position.x;
				float const top = start_pos.y + glyph_info.position.y;
				float const right = left + glyph_info.size.x;
				float const bottom = top - glyph_info.size.y;
				rect.a.x = std::min(rect.a.x, left);
				rect.a.y = std::max(rect.a.y, top);
				rect.b.x = std::max(rect.b.x, right);
				rect.b.y = std::min(rect.b.y, bottom);
				is_updated = true;
				// 前进
				start_pos += glyph_info.advance;
			}
		}

		if (is_updated)
		{
			rect.a.x *= m_scale.x;
			rect.a.y *= m_scale.y;
			rect.b.x *= m_scale.x;
			rect.b.y *= m_scale.y;
			return rect;
		}
		else
		{
			return RectF();
		}
	}
	Vector2F TextRenderer_D3D11::getTextAdvance(StringView str)
	{
		if (!m_glyphmgr)
		{
			return Vector2F();
		}

		utf::utf8reader reader_(str.data(), str.size());

		GlyphInfo glyph_info = {};
		Vector2F start_pos;
		bool is_updated = false;
		float const line_height = m_glyphmgr->getLineHeight();

		char32_t code_ = 0;
		while (reader_(code_))
		{
			if (code_ == U'\n')
			{
				start_pos.x = 0.0f;
				start_pos.y -= line_height;
				continue;
			}
			if (m_glyphmgr->getGlyph(code_, &glyph_info, true))
			{
				is_updated = true;
				start_pos += glyph_info.advance;
			}
		}

		if (is_updated)
		{
			start_pos.x *= m_scale.x;
			start_pos.y *= m_scale.y;
			return start_pos;
		}
		else
		{
			return Vector2F();
		}
	}
	bool TextRenderer_D3D11::drawText(StringView str, Vector2F const& start, Vector2F* endout)
	{
		if (!m_glyphmgr)
		{
			return false;
		}

		// 首先，缓存所有字形
		if (!m_glyphmgr->cacheString(str))
		{
			//return false; // 找不到的就忽略
		}
		if (!m_glyphmgr->flush())
		{
			return false;
		}

		// 绘制参数
		GlyphInfo glyph_info = {};
		Vector2F start_pos = start; // 笔触位置
		float const line_height = m_glyphmgr->getLineHeight() * m_scale.y; // 行高

		// 比较常用的空格
		GlyphInfo space_glyph_info = {};
		if (!m_glyphmgr->getGlyph(uint32_t(U' '), &space_glyph_info, true))
		{
			//assert(false); return false; // 没有空格就默认它所有度量值为 0
		}
		space_glyph_info.advance.x *= m_scale.x;
		space_glyph_info.advance.y *= m_scale.y;

		// 迭代绘制所有文字
		if (m_glyphmgr->getTextureCount() <= 1)
		{
			char32_t code_ = 0;
			utf::utf8reader reader_(str.data(), str.size());
			while (reader_(code_))
			{
				// 换行处理
				if (code_ == U'\n')
				{
					start_pos.x = start.x;
					start_pos.y -= line_height; // 向下换行
					continue;
				}
				
				// 空格处理
				if (code_ == U' ')
				{
					start_pos += space_glyph_info.advance;
					continue;
				}

				// 取出文字
				if (m_glyphmgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
				{
					// 绘制字形
					glyph_info.size.x *= m_scale.x;
					glyph_info.size.y *= m_scale.y;
					glyph_info.position.x *= m_scale.x;
					glyph_info.position.y *= m_scale.y;
					if (!drawGlyph(glyph_info, start_pos)) return false;

					// 前进
					glyph_info.advance.x *= m_scale.x;
					glyph_info.advance.y *= m_scale.y;
					start_pos += glyph_info.advance;
				}
			}
		}
		else
		{
			// 迭代所有纹理
			for (uint32_t idx = 0; idx < m_glyphmgr->getTextureCount(); idx += 1)
			{
				start_pos = start; // 重置笔触位置
				char32_t code_ = 0;
				utf::utf8reader reader_(str.data(), str.size());
				while (reader_(code_))
				{
					// 换行处理
					if (code_ == U'\n')
					{
						start_pos.x = start.x;
						start_pos.y -= line_height; // 向下换行
						continue;
					}

					// 空格处理
					if (code_ == U' ')
					{
						start_pos += space_glyph_info.advance;
						continue;
					}

					// 取出文字
					if (m_glyphmgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
					{
						// 绘制字形
						if (glyph_info.texture_index == idx)
						{
							glyph_info.size.x *= m_scale.x;
							glyph_info.size.y *= m_scale.y;
							glyph_info.position.x *= m_scale.x;
							glyph_info.position.y *= m_scale.y;
							if (!drawGlyph(glyph_info, start_pos)) return false;
						}

						// 前进
						glyph_info.advance.x *= m_scale.x;
						glyph_info.advance.y *= m_scale.y;
						start_pos += glyph_info.advance;
					}
				}
			}
		}

		// 返回新的笔触位置
		if (endout)
		{
			*endout = start_pos;
		}

		return true;
	}
	bool TextRenderer_D3D11::drawTextInSpace(StringView str,
		Vector3F const& start, Vector3F const& right_vec, Vector3F const& down_vec,
		Vector3F* endout)
	{
		if (!m_glyphmgr)
		{
			assert(false); return false;
		}

		// 首先，缓存所有字形
		if (!m_glyphmgr->cacheString(str))
		{
			//return false; // 找不到的就忽略
		}
		if (!m_glyphmgr->flush())
		{
			return false;
		}

		// 绘制参数
		GlyphInfo glyph_info = {};
		Vector3F line_pos = start; // 行起始笔触位置
		Vector3F start_pos = start; // 笔触位置
		float const line_height = m_glyphmgr->getLineHeight() * m_scale.y; // 行高

		// 比较常用的空格
		GlyphInfo space_glyph_info = {};
		if (!m_glyphmgr->getGlyph(uint32_t(U' '), &space_glyph_info, true))
		{
			//assert(false); return false; // 没有空格就默认它所有度量值为 0
		}
		space_glyph_info.advance.x *= m_scale.x;
		space_glyph_info.advance.y *= m_scale.y;

		// 迭代绘制所有文字
		if (m_glyphmgr->getTextureCount() <= 1)
		{
			char32_t code_ = 0;
			utf::utf8reader reader_(str.data(), str.size());
			while (reader_(code_))
			{
				// 换行处理
				if (code_ == U'\n')
				{
					line_pos += down_vec * line_height; // 向下换行
					start_pos = line_pos; // 回到行起点
					continue;
				}

				// 空格处理
				if (code_ == U' ')
				{
					start_pos = start_pos + (right_vec * space_glyph_info.advance.x) + (down_vec * space_glyph_info.advance.y);
					continue;
				}

				// 取出文字
				if (m_glyphmgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
				{
					// 绘制
					glyph_info.size.x *= m_scale.x;
					glyph_info.size.y *= m_scale.y;
					glyph_info.position.x *= m_scale.x;
					glyph_info.position.y *= m_scale.y;
					if (!drawGlyphInSpace(glyph_info, start_pos, right_vec, down_vec)) return false;

					// 前进
					glyph_info.advance.x *= m_scale.x;
					glyph_info.advance.y *= m_scale.y;
					start_pos = start_pos + (right_vec * glyph_info.advance.x) + (down_vec * glyph_info.advance.y);
				}
			}
		}
		else
		{
			// 迭代所有纹理
			for (uint32_t idx = 0; idx < m_glyphmgr->getTextureCount(); idx += 1)
			{
				line_pos = start; // 重置行起始笔触位置
				start_pos = start; // 重置笔触位置
				char32_t code_ = 0;
				utf::utf8reader reader_(str.data(), str.size());
				while (reader_(code_))
				{
					// 换行处理
					if (code_ == U'\n')
					{
						line_pos += down_vec * line_height; // 向下换行
						start_pos = line_pos; // 回到行起点
						continue;
					}

					// 空格处理
					if (code_ == U' ')
					{
						start_pos = start_pos + (right_vec * space_glyph_info.advance.x) + (down_vec * space_glyph_info.advance.y);
						continue;
					}

					// 取出文字
					if (m_glyphmgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
					{
						// 绘制
						if (glyph_info.texture_index == idx)
						{
							glyph_info.size.x *= m_scale.x;
							glyph_info.size.y *= m_scale.y;
							glyph_info.position.x *= m_scale.x;
							glyph_info.position.y *= m_scale.y;
							if (!drawGlyphInSpace(glyph_info, start_pos, right_vec, down_vec)) return false;
						}

						// 前进
						glyph_info.advance.x *= m_scale.x;
						glyph_info.advance.y *= m_scale.y;
						start_pos = start_pos + (right_vec * glyph_info.advance.x) + (down_vec * glyph_info.advance.y);
					}
				}
			}
		}

		// 返回新的笔触位置
		if (endout)
		{
			*endout = start_pos;
		}

		return true;
	}

	TextRenderer_D3D11::TextRenderer_D3D11(IRenderer* p_renderer)
		: m_renderer(p_renderer)
		, m_scale(Vector2F(1.0f, 1.0f))
		, m_z(0.5f)
		, m_color(Color4B(0xFFFFFFFFu))
	{
	}
	TextRenderer_D3D11::~TextRenderer_D3D11()
	{
	}

	bool ITextRenderer::create(IRenderer* p_renderer, ITextRenderer** pp_textrenderer)
	{
		try
		{
			*pp_textrenderer = new TextRenderer_D3D11(p_renderer);
			return true;
		}
		catch (...)
		{
			*pp_textrenderer = nullptr;
			return false;
		}
	}
}

#include <ShlObj.h>

bool findSystemFont(std::string_view name, std::string& u8_path)
{
	std::wstring wide_name(std::move(utility::encoding::to_wide(name)));

	// 打开注册表 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
	// 枚举符合要求的字体
	HKEY tKey = NULL;
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &tKey) == ERROR_SUCCESS)
	{
		// 枚举子键
		int tIndex = 0;
		WCHAR tKeyName[MAX_PATH]{};
		DWORD tKeyNameLen = MAX_PATH;
		DWORD tKeyType = 0;
		BYTE tKeyData[MAX_PATH]{};
		DWORD tKeyDataLen = MAX_PATH;

		while (RegEnumValueW(tKey, tIndex, tKeyName, &tKeyNameLen, NULL, &tKeyType, tKeyData, &tKeyDataLen) == ERROR_SUCCESS)
		{
			// 检查是否为相应字体
			if (tKeyType == REG_SZ)
			{
				WCHAR tFontName[MAX_PATH]{};
				WCHAR tFontType[MAX_PATH]{};
				if (2 == swscanf_s(tKeyName, L"%[^()] (%[^()])", tFontName, MAX_PATH, tFontType, MAX_PATH))
				{
					size_t tLen = wcslen(tFontName);

					// 去除scanf匹配的空格
					if (!tLen)
						continue;
					else
					{
						if (tFontName[tLen - 1] == L' ')
							tFontName[tLen - 1] = L'\0';
					}
					// 是否为需要的字体
					if (tFontName == wide_name)
					{
						RegCloseKey(tKey);

						WCHAR tTextDir[MAX_PATH]{};
						SHGetSpecialFolderPathW(GetDesktopWindow(), tTextDir, CSIDL_FONTS, 0);
						std::wstring tPath = tTextDir;
						tPath += L'\\';
						tPath += (WCHAR*)tKeyData;

						// 返回路径
						u8_path = std::move(utility::encoding::to_utf8(tPath));
						return true;
					}
				}
			}

			// 继续枚举
			tKeyNameLen = MAX_PATH;
			tKeyType = 0;
			tKeyDataLen = MAX_PATH;
			tIndex++;
		}
	}
	else
	{
		return false; // 打开注册表失败
	}

	RegCloseKey(tKey);

	return false; // 没找到
}
