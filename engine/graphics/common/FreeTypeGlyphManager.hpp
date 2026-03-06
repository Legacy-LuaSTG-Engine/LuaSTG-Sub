#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/Graphics/Font.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

namespace core {
	class FreeTypeGlyphManager final
		: public implement::ReferenceCounted<Graphics::IGlyphManager>
		, public IGraphicsDeviceEventListener {
	public:
		// IGraphicsDeviceEventListener

		void onGraphicsDeviceCreate() override;
		void onGraphicsDeviceDestroy() override;

		// IGlyphManager

		float getLineHeight() override;
		float getAscender() override;
		float getDescender() override;

		uint32_t getTextureCount() override;
		ITexture2D* getTexture(uint32_t index) override;

		bool cacheGlyph(uint32_t codepoint) override;
		bool cacheString(StringView str) override;
		bool flush() override;

		bool getGlyph(uint32_t codepoint, Graphics::GlyphInfo* p_ref_info, bool no_render) override;

		// FreeTypeGlyphManager

		FreeTypeGlyphManager(IGraphicsDevice* p_device, Graphics::TrueTypeFontInfo const* p_arr_info, size_t info_count);
		FreeTypeGlyphManager(FreeTypeGlyphManager const&) = delete;
		FreeTypeGlyphManager(FreeTypeGlyphManager&&) = delete;
		~FreeTypeGlyphManager();

		FreeTypeGlyphManager& operator=(FreeTypeGlyphManager const&) = delete;
		FreeTypeGlyphManager& operator=(FreeTypeGlyphManager&&) = delete;

	private:
		struct Image2D {
			static constexpr uint32_t texture_size{ 1024 };
			uint32_t width{ texture_size };
			uint32_t height{ texture_size };
			uint32_t pitch{ texture_size * sizeof(Color4B) };
			Color4B data[texture_size * texture_size];

			Color4B& pixel(uint32_t const x, uint32_t const y) {
				return data[y * texture_size + x];
			}

			Image2D();
		};

		struct GlyphCache2D {
			static constexpr uint32_t invalid_rect_value{ 0x7FFFFFFF };
			Image2D image;
			SmartReference<ITexture2D> texture;
			uint32_t pen_x{};
			uint32_t pen_y{};
			uint32_t pen_bottom{};
			uint32_t dirty_l{ invalid_rect_value };
			uint32_t dirty_t{ invalid_rect_value };
			uint32_t dirty_r{ invalid_rect_value };
			uint32_t dirty_b{ invalid_rect_value };
		};

		struct GlyphCacheInfo {
			// 继承自 GlyphInfo
			uint32_t texture_index = 0; // 字形在哪个纹理上
			RectF    texture_rect;      // 字形在纹理上的uv坐标
			Vector2F size;              // 字形大小
			Vector2F position;          // 笔触距离字形左上角坐标
			Vector2F advance;           // 前进量
			// 私有
			uint32_t codepoint = 0;     // 当前的字符
		};

		struct FreeTypeFontData {
			SmartReference<IData> buffer;
			FT_Face ft_face{};
			float ft_line_height{};
			float ft_ascender{};
			float ft_descender{};
			uint32_t is_fallback{};
		};

		struct FreeTypeFontCommonInfo {
			float ft_line_height{};
			float ft_ascender{};
			float ft_descender{};
		};

		void closeFonts();
		bool openFonts(Graphics::TrueTypeFontInfo const* fonts, size_t count);
		bool addTexture();
		bool findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index) const;
		bool writeBitmapToCache(GlyphCacheInfo& info, FT_Bitmap const& bitmap);
		GlyphCacheInfo* getGlyphCacheInfo(uint32_t codepoint);
		bool renderCache(uint32_t codepoint);

		SmartReference<IGraphicsDevice> m_device;
		FreeTypeFontCommonInfo m_common_info;
		std::vector<FreeTypeFontData> m_font;
		std::vector<GlyphCache2D> m_tex;
		std::unordered_map<uint32_t, GlyphCacheInfo> m_map;
	};
}
