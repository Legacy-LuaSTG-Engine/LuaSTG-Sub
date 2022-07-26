#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Font.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

namespace Core::Graphics
{
	constexpr uint32_t const TEXTURE_SIZE = 1024;
	constexpr uint32_t const INVALID_RECT = 0x7FFFFFFF;

	struct Image2D
	{
		uint32_t const width = TEXTURE_SIZE;
		uint32_t const height = TEXTURE_SIZE;
		uint32_t const pitch = TEXTURE_SIZE * sizeof(Color4B);
		Color4B data[TEXTURE_SIZE * TEXTURE_SIZE];
		inline Color4B& pixel(uint32_t x, uint32_t y)
		{
			return data[y * 1024 + x];
		}
		Image2D();
	};

	struct GlyphCache2D
	{
		Image2D image;
		ScopeObject<ITexture2D> texture;
		uint32_t pen_x = 0;
		uint32_t pen_y = 0;
		uint32_t pen_bottom = 0;
		uint32_t dirty_l = INVALID_RECT;
		uint32_t dirty_t = INVALID_RECT;
		uint32_t dirty_r = INVALID_RECT;
		uint32_t dirty_b = INVALID_RECT;
	};

	struct GlyphCacheInfo
	{
		// 继承自 GlyphInfo
		uint32_t texture_index = 0; // 字形在哪个纹理上
		RectF    texture_rect;      // 字形在纹理上的uv坐标
		Vector2F size;              // 字形大小
		Vector2F position;          // 笔触距离字形左上角坐标
		Vector2F advance;           // 前进量
		// 私有
		uint32_t codepoint = 0;     // 当前的字符
	};

	struct FreeTypeFontData
	{
		std::vector<uint8_t> buffer;
		FT_Face ft_face{ NULL };
		float ft_line_height{ 0.0f };
		float ft_ascender{ 0.0f };
		float ft_descender{ 0.0f };
		uint32_t is_fallback{ 0 };
	};

	struct FreeTypeFontCommonInfo
	{
		float ft_line_height{ 0.0f };
		float ft_ascender{ 0.0f };
		float ft_descender{ 0.0f };
	};

	class TrueTypeGlyphManager_D3D11
		: public Object<IGlyphManager>
		, public IDeviceEventListener
	{
	private:
		ScopeObject<IDevice> m_device;
		FreeTypeFontCommonInfo m_common_info;
		std::vector<FreeTypeFontData> m_font;
		std::vector<GlyphCache2D> m_tex;
		std::unordered_map<uint32_t, GlyphCacheInfo> m_map;

	public:
		void onDeviceCreate();
		void onDeviceDestroy();

	private:
		void closeFonts();
		bool openFonts(TrueTypeFontInfo* fonts, size_t count);
		bool addTexture();
		bool findGlyph(FT_ULong code, FT_Face& face, FT_UInt& index);
		bool writeBitmapToCache(GlyphCacheInfo& info, FT_Bitmap& bitmap);
		GlyphCacheInfo* getGlyphCacheInfo(uint32_t codepoint);
		bool renderCache(uint32_t codepoint);

	public:
		float getLineHeight();
		float getAscender();
		float getDescender();

		uint32_t getTextureCount();
		ITexture2D* getTexture(uint32_t index);

		bool cacheGlyph(uint32_t codepoint);
		bool cacheString(StringView str);
		bool flush();

		bool getGlyph(uint32_t codepoint, GlyphInfo* p_ref_info, bool no_render);

	public:
		TrueTypeGlyphManager_D3D11(IDevice* p_device, TrueTypeFontInfo* p_arr_info, size_t info_count);
		~TrueTypeGlyphManager_D3D11();
	};

	class TextRenderer_D3D11
		: public Object<ITextRenderer>
	{
	private:
		ScopeObject<IRenderer> m_renderer;
		ScopeObject<IGlyphManager> m_glyphmgr;
		Vector2F m_scale;
		float m_z{ 0.0f };
		Color4B m_color;

	private:
		bool drawGlyph(GlyphInfo const& glyph_info, Vector2F const& start_pos);
		bool drawGlyphInSpace(GlyphInfo const& glyph_info, Vector3F const& start_pos, Vector3F const& right_vec, Vector3F const& down_vec);

	public:
		void setScale(Vector2F const& scale) { m_scale = scale; }
		Vector2F getScale() { return m_scale; }
		void setColor(Color4B const color) { m_color = color; }
		Color4B getColor() { return m_color; }
		void setZ(float const z) { m_z = z; }
		float getZ() { return m_z; }
		void setGlyphManager(IGlyphManager* p_mgr) { m_glyphmgr = p_mgr; }
		IGlyphManager* getGlyphManager() { return m_glyphmgr.get(); }

		RectF getTextBoundary(StringView str);
		Vector2F getTextAdvance(StringView str);
		bool drawText(StringView str, Vector2F const& start, Vector2F* endout);
		bool drawTextInSpace(StringView str,
			Vector3F const& start, Vector3F const& right_vec, Vector3F const& down_vec,
			Vector3F* endout);

	public:
		TextRenderer_D3D11(IRenderer* p_renderer);
		~TextRenderer_D3D11();
	};
}
