#include "Core/Graphics/Common/FreeTypeGlyphManager.hpp"
#include "Core/FileManager.hpp"
#include "utility/utf.hpp"
#include "utf8.hpp"

namespace {
	bool findSystemFont(std::string_view name, std::string& u8_path);

	// 宽度单位到像素
	float widthSizeToPixel(FT_Face const face, int const size) {
		double const scale = static_cast<double>(face->size->metrics.x_scale) / 65536.0;
		return static_cast<float>((static_cast<double>(size) / 64.0) * scale);
	}
	// 高度单位到像素
	float heightSizeToPixel(FT_Face const face, int const size) {
		double const scale = static_cast<double>(face->size->metrics.y_scale) / 65536.0;
		return static_cast<float>((static_cast<double>(size) / 64.0) * scale);
	}

	class FreeTypeBitmapAccessor {
	public:
		explicit FreeTypeBitmapAccessor(FT_Bitmap const& bitmap) noexcept : m_bitmap(&bitmap) {}
		FreeTypeBitmapAccessor(FreeTypeBitmapAccessor const&) = delete;
		FreeTypeBitmapAccessor(FreeTypeBitmapAccessor&&) = delete;
		~FreeTypeBitmapAccessor() = default;

		FreeTypeBitmapAccessor& operator=(FreeTypeBitmapAccessor const&) = delete;
		FreeTypeBitmapAccessor& operator=(FreeTypeBitmapAccessor&&) = delete;

		[[nodiscard]] uint32_t width() const noexcept { return m_bitmap->width; }
		[[nodiscard]] uint32_t height() const noexcept { return m_bitmap->rows; }
		[[nodiscard]] uint32_t pixel(uint32_t const x, uint32_t const y) const noexcept {
			// FT_Bitmap::pitch 是有符号的，也许有负数的可能性？
			if (m_bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
				auto const line = static_cast<uint8_t const*>(m_bitmap->buffer) + (static_cast<ptrdiff_t>(y) * static_cast<ptrdiff_t>(m_bitmap->pitch));
				return (line[x] << 24) | 0x00FFFFFF;
			}
			if (m_bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
				auto const line = static_cast<uint8_t const*>(m_bitmap->buffer) + (static_cast<ptrdiff_t>(y) * static_cast<ptrdiff_t>(m_bitmap->pitch));
				auto const block = line[x / 8];
				auto const flag = (1 << (7 - (x % 8))) & block; // 最左边的像素在最高位
				return flag ? 0xFFFFFFFF : 0x00FFFFFF;
			}
			return 0;
		}

	private:
		FT_Bitmap const* m_bitmap{};
	};

	class FreeTypeLibrarySingleton {
	public:
		FreeTypeLibrarySingleton() {
			if (FT_Err_Ok != FT_Init_FreeType(&m_library)) {
				throw std::runtime_error("FT_Init_FreeType failed.");
			}
		}
		FreeTypeLibrarySingleton(FreeTypeLibrarySingleton const&) = delete;
		FreeTypeLibrarySingleton(FreeTypeLibrarySingleton&&) = delete;
		~FreeTypeLibrarySingleton() {
			if (m_library) {
				FT_Done_FreeType(m_library);
				m_library = nullptr;
			}
		}

		FreeTypeLibrarySingleton& operator=(FreeTypeLibrarySingleton const&) = delete;
		FreeTypeLibrarySingleton& operator=(FreeTypeLibrarySingleton&&) = delete;

		[[nodiscard]] FT_Library get() const noexcept { return m_library; }

		static FreeTypeLibrarySingleton& getInstance() {
			static FreeTypeLibrarySingleton instance;
			return instance;
		}

	private:
		FT_Library m_library{};
	};

#define FT_LIBRARY (FreeTypeLibrarySingleton::getInstance().get())
}

namespace Core::Graphics::Common {
	Image2D::Image2D() {
		std::memset(data, 0, sizeof(data));
	}
}
namespace Core::Graphics::Common {
	// IDeviceEventListener

	void FreeTypeGlyphManager::onDeviceCreate() {
		// 等一个幸运儿调用 flush
	}
	void FreeTypeGlyphManager::onDeviceDestroy() {
		// 全标记为脏
		for (auto& t : m_tex) {
			t.dirty_l = 0;
			t.dirty_t = 0;
			t.dirty_r = t.image.width;
			t.dirty_b = t.pen_bottom + 1; // 多一个像素的边缘
		}
	}

	// IGlyphManager

	float FreeTypeGlyphManager::getLineHeight() {
		return m_common_info.ft_line_height;
	}
	float FreeTypeGlyphManager::getAscender() {
		return m_common_info.ft_ascender;
	}
	float FreeTypeGlyphManager::getDescender() {
		return m_common_info.ft_descender;
	}

	uint32_t FreeTypeGlyphManager::getTextureCount() {
		return static_cast<uint32_t>(m_tex.size());
	}
	ITexture2D* FreeTypeGlyphManager::getTexture(uint32_t const index) {
		if (index < m_tex.size()) {
			return m_tex[index].texture.get();
		}
		assert(false); return nullptr;
	}

	bool FreeTypeGlyphManager::cacheGlyph(uint32_t const codepoint) {
		return getGlyphCacheInfo(codepoint) != nullptr;
	}
	bool FreeTypeGlyphManager::cacheString(StringView const str) {
		// utf-8 迭代器
		char32_t codepoint{};
		utf::utf8reader reader(str.data(), str.size());
		bool result = true;
		while (reader(codepoint)) {
			if (!getGlyphCacheInfo(codepoint))
				if (codepoint != U'\n')
					result = false;
		}
		return result;
	}
	bool FreeTypeGlyphManager::flush() {
		for (auto& t : m_tex) {
			if (t.dirty_l != GlyphCache2D::invalid_rect_value) {
				if (!t.texture->uploadPixelData(
					RectU(t.dirty_l, t.dirty_t, t.dirty_r, t.dirty_b),
					&t.image.pixel(t.dirty_l, t.dirty_t),
					t.image.pitch
				)) {
					return false;
				}
				t.dirty_l = GlyphCache2D::invalid_rect_value;
				t.dirty_t = GlyphCache2D::invalid_rect_value;
				t.dirty_r = GlyphCache2D::invalid_rect_value;
				t.dirty_b = GlyphCache2D::invalid_rect_value;
			}
		}
		return true;
	}

	bool FreeTypeGlyphManager::getGlyph(uint32_t const codepoint, GlyphInfo* const p_ref_info, bool const no_render) {
		if (!p_ref_info) {
			assert(false); return false;
		}
		if (GlyphCacheInfo const* const info = getGlyphCacheInfo(codepoint)) {
			p_ref_info->texture_index = info->texture_index;
			p_ref_info->texture_rect = info->texture_rect;
			p_ref_info->size = info->size;
			p_ref_info->position = info->position;
			p_ref_info->advance = info->advance;
			if (no_render) {
				return true;
			}
			return flush();
		}
		return false;
	}

	// FreeTypeGlyphManager

	FreeTypeGlyphManager::FreeTypeGlyphManager(IDevice* const p_device, TrueTypeFontInfo const* const p_arr_info, size_t const info_count)
		: m_device(p_device) {
		if (!openFonts(p_arr_info, info_count)) {
			throw std::runtime_error("FreeTypeGlyphManager::FreeTypeGlyphManager (openFonts)");
		}
		if (!addTexture()) {
			throw std::runtime_error("FreeTypeGlyphManager::FreeTypeGlyphManager (addTexture)");
		}
		cacheGlyph(U' '); // 先缓存一个空格
		m_device->addEventListener(this);
	}
	FreeTypeGlyphManager::~FreeTypeGlyphManager() {
		m_device->removeEventListener(this);
		closeFonts();
	}

	void FreeTypeGlyphManager::closeFonts() {
		// 先关闭字体
		for (auto& f : m_font) {
			if (f.ft_face) {
				FT_Done_Face(f.ft_face);
				f.ft_face = nullptr;
			}
		}
		m_font.clear();
	}
	bool FreeTypeGlyphManager::openFonts(TrueTypeFontInfo const* const fonts, size_t const count) {
		closeFonts();
		if (!fonts || count == 0) {
			assert(false); return false;
		}

		// 逐个打开字体
		m_font.resize(count);
		for (size_t i = 0; i < count; i++) {
			// 准备数据
			FreeTypeFontData& data = m_font[i];
			auto setupFontFace = [&]() {
				// 设置一些参数
				if (FT_Err_Ok != FT_Set_Pixel_Sizes(data.ft_face, (FT_UInt)fonts[i].font_size.x, (FT_UInt)fonts[i].font_size.y)) {
					FT_Done_Face(data.ft_face);
					data.ft_face = NULL;
					return;
				}
				// 计算一些度量值
				data.ft_line_height = heightSizeToPixel(data.ft_face, data.ft_face->height);
				data.ft_ascender = heightSizeToPixel(data.ft_face, data.ft_face->ascender);
				data.ft_descender = heightSizeToPixel(data.ft_face, data.ft_face->descender);
			};
			auto openFromFile = [&](std::string_view path) {
				// 打开
				if (FT_Err_Ok != FT_New_Face(FT_LIBRARY, path.data(), (FT_Long)fonts[i].font_face, &data.ft_face)) {
					return;
				}
				setupFontFace();
			};
			auto openFromBuffer = [&]() {
				// 打开
				if (FT_Err_Ok != FT_New_Memory_Face(FT_LIBRARY, (FT_Byte*)data.buffer.data(), (FT_Long)data.buffer.size(), (FT_Long)fonts[i].font_face, &data.ft_face)) {
					return;
				}
				setupFontFace();
			};
			if (!fonts[i].is_buffer) {
				// 先看看是不是要从系统加载
				std::string u8_path(fonts[i].source);
				if (!GFileManager().containEx(u8_path)) {
					if (!findSystemFont(fonts[i].source, u8_path)) {
						continue; // 还是找不到
					}
				}
				// 如果是路径，尝试从文件打开
				if (!fonts[i].is_force_to_file) {
					// 读取进内存
					if (GFileManager().loadEx(u8_path, data.buffer)) {
						openFromBuffer();
					}
				} else {
					// 从文件打开
					openFromFile(u8_path);
				}
			} else if (fonts[i].source.data() && fonts[i].source.size() > 0) {
				// 如果是一块二进制数据，复制下来备用
				data.buffer.resize(fonts[i].source.size());
				std::memcpy(data.buffer.data(), fonts[i].source.data(), fonts[i].source.size());
				openFromBuffer();
			} else {
				assert(false); continue; // 您数据呢？
			}
		}

		// 检查结果
		bool is_all_open = true;
		for (auto const& f : m_font) {
			if (f.ft_face == nullptr) {
				is_all_open = false;
				break;
			}
		}
		if (is_all_open) {
			// 设置最后一个字体为回落字体
			m_font.back().is_fallback = true;
			// 计算公共参数
			for (auto& f : m_font) {
				m_common_info.ft_line_height = std::max(m_common_info.ft_line_height, f.ft_line_height);
				m_common_info.ft_ascender = std::max(m_common_info.ft_ascender, f.ft_ascender);
				m_common_info.ft_descender = std::min(m_common_info.ft_descender, f.ft_descender);
			}
			return true;
		} else {
			closeFonts();
			return false;
		}
	}
	bool FreeTypeGlyphManager::addTexture() {
		m_tex.emplace_back();
		// ReSharper disable once CppTooWideScopeInitStatement
		auto& t = m_tex.back();
		if (!m_device->createTexture(Vector2U(t.image.width, t.image.height), ~t.texture)) {
			return false;
		}
		//t.texture->setPremultipliedAlpha(true); // 为了支持彩色文本，需要使用预乘 alpha 模式
		return true;
	}
	bool FreeTypeGlyphManager::findGlyph(FT_ULong const code, FT_Face& face, FT_UInt& index) const {
		for (auto const& f : m_font) {
			if (f.ft_face) {
				if (FT_UInt const i = FT_Get_Char_Index(f.ft_face, code); i != 0 || f.is_fallback) {
					face = f.ft_face;
					index = i;
					return true;
				}
			}
		}
		return false;
	}
	bool FreeTypeGlyphManager::writeBitmapToCache(GlyphCacheInfo& info, FT_Bitmap const& bitmap) {
		// 太大的不要，滚
		if (bitmap.width > (Image2D::texture_size - 2) || bitmap.rows > (Image2D::texture_size - 2)) {
			assert(false); return false;
		}
		// 搞到一个位置
		GlyphCache2D* pt = nullptr;
		do {
			for (auto& t : m_tex) {
				// 初始化，留出 1 像素左上边缘
				if (t.pen_x == 0 || t.pen_y == 0) {
					t.pen_x = 1;
					t.pen_y = 1;
				}
				// 这行能塞下吗，留出 1 像素右下边缘
				if ((t.pen_x + bitmap.width) < (t.image.width - 1) && (t.pen_y + bitmap.rows) < (t.image.height - 1)) {
					pt = &t;
					break;
				}
				// 换行，留出 1 像素行边缘
				uint32_t const new_pen_x = 1;
				uint32_t const new_pen_y = std::max(t.pen_y, t.pen_bottom + 1);
				// 这行能塞下吗，留出 1 像素右下边缘
				if ((new_pen_x + bitmap.width) < (t.image.width - 1) && (new_pen_y + bitmap.rows) < (t.image.height - 1)) {
					t.pen_x = new_pen_x;
					t.pen_y = new_pen_y;
					pt = &t;
					break;
				}
			}
			if (pt) {
				break;
			} else {
				// 该添新丁了
				if (!addTexture()) {
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
		FreeTypeBitmapAccessor const accessor(bitmap);
		for (int x = 0; x < (int)(accessor.width() + 2); x += 1) // 上 1 像素宽的边，宽度是 bitmap 的宽度再加 2 像素
		{
			t.image.pixel(t.pen_x - 1 + x, t.pen_y - 1) = Color4B(0x00FFFFFF);
		}
		for (int y = 0; y < (int)accessor.height(); y += 1) {
			t.image.pixel(t.pen_x - 1, t.pen_y + y) = Color4B(0x00FFFFFF); // 左 1 像素宽的边
			for (int x = 0; x < (int)accessor.width(); x += 1) {
				t.image.pixel(t.pen_x + x, t.pen_y + y) = accessor.pixel(x, y);
			}
			t.image.pixel(t.pen_x + accessor.width(), t.pen_y + y) = Color4B(0x00FFFFFF); // 右 1 像素宽的边
		}
		for (int x = 0; x < (int)(accessor.width() + 2); x += 1) // 下 1 像素宽的边，宽度是 bitmap 的宽度再加 2 像素
		{
			t.image.pixel(t.pen_x - 1 + x, t.pen_y + accessor.height()) = Color4B(0x00FFFFFF);
		}
		// 更新脏区域
		if (t.dirty_l == GlyphCache2D::invalid_rect_value) {
			t.dirty_l = t.pen_x - 1;
			t.dirty_t = t.pen_y - 1;
			t.dirty_r = t.pen_x + bitmap.width + 1;
			t.dirty_b = t.pen_y + bitmap.rows + 1;
		} else {
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
	GlyphCacheInfo* FreeTypeGlyphManager::getGlyphCacheInfo(uint32_t const codepoint) {
		auto const it = m_map.find(codepoint);
		if (it != m_map.end()) {
			return &it->second;
		}
		if (renderCache(codepoint)) {
			return &m_map[codepoint];
		}
		return nullptr;
	}
	bool FreeTypeGlyphManager::renderCache(uint32_t const codepoint) {
		FT_Face face{};
		// ReSharper disable once CppTooWideScopeInitStatement
		FT_UInt index{};
		if (findGlyph(codepoint, face, index)) {
			// 加载文字到字形槽并渲染
			FT_Load_Glyph(face, index, FT_LOAD_RENDER); // 需要处理错误吗？
			FT_GlyphSlot const& glyph = face->glyph;
			FT_Bitmap const& bitmap = glyph->bitmap;
			// 写入对应属性
			GlyphCacheInfo cache = {};
			cache.size = Vector2F(static_cast<float>(glyph->bitmap.width), static_cast<float>(glyph->bitmap.rows));
			cache.position = Vector2F(static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top));
			cache.advance = Vector2F(static_cast<float>(glyph->advance.x) / 64.f, static_cast<float>(glyph->advance.y) / 64.f);
			cache.codepoint = codepoint;
			if (!writeBitmapToCache(cache, bitmap)) {
				return false;
			}
			m_map.emplace(codepoint, cache);
		}
		return false;
	}
}
namespace Core::Graphics {
	bool IGlyphManager::create(IDevice* const p_device, TrueTypeFontInfo const* const p_arr_info, size_t const info_count, IGlyphManager** const output) {
		try {
			*output = new Common::FreeTypeGlyphManager(p_device, p_arr_info, info_count);
			return true;
		} catch (...) {
			*output = nullptr;
			return false;
		}
	}
}

// TODO: move to Platform/Windows folder

#include <ShlObj.h>

namespace {
	bool findSystemFont(std::string_view const name, std::string& u8_path) {
		std::wstring wide_name(utf8::to_wstring(name));

		// 打开注册表 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
		// 枚举符合要求的字体
		HKEY tKey = NULL;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &tKey) == ERROR_SUCCESS) {
			// 枚举子键
			int tIndex = 0;
			WCHAR tKeyName[MAX_PATH]{};
			DWORD tKeyNameLen = MAX_PATH;
			DWORD tKeyType = 0;
			BYTE tKeyData[MAX_PATH]{};
			DWORD tKeyDataLen = MAX_PATH;

			while (RegEnumValueW(tKey, tIndex, tKeyName, &tKeyNameLen, NULL, &tKeyType, tKeyData, &tKeyDataLen) == ERROR_SUCCESS) {
				// 检查是否为相应字体
				if (tKeyType == REG_SZ) {
					WCHAR tFontName[MAX_PATH]{};
					WCHAR tFontType[MAX_PATH]{};
					if (2 == swscanf_s(tKeyName, L"%[^()] (%[^()])", tFontName, MAX_PATH, tFontType, MAX_PATH)) {
						size_t tLen = wcslen(tFontName);

						// 去除scanf匹配的空格
						if (!tLen)
							continue;
						else {
							if (tFontName[tLen - 1] == L' ')
								tFontName[tLen - 1] = L'\0';
						}
						// 是否为需要的字体
						if (tFontName == wide_name) {
							RegCloseKey(tKey);

							WCHAR tTextDir[MAX_PATH]{};
							SHGetSpecialFolderPathW(GetDesktopWindow(), tTextDir, CSIDL_FONTS, 0);
							std::wstring tPath = tTextDir;
							tPath += L'\\';
							tPath += (WCHAR*)tKeyData;

							// 返回路径
							u8_path = utf8::to_string(tPath);
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
		} else {
			return false; // 打开注册表失败
		}

		RegCloseKey(tKey);

		return false; // 没找到
	}
}
