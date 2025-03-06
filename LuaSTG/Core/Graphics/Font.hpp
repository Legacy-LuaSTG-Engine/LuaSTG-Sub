#pragma once
#include "Core/Graphics/Device.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace Core::Graphics
{
	struct GlyphInfo
	{
		uint32_t texture_index = 0; // 字形在哪个纹理上
		RectF    texture_rect;      // 字形在纹理上的uv坐标
		Vector2F size;              // 字形大小
		Vector2F position;          // 笔触距离字形左上角坐标
		Vector2F advance;           // 前进量
	};

	struct TrueTypeFontInfo
	{
		StringView source;
		uint32_t   font_face;        // 字面索引，一个字体文件中有多个字面
		Vector2F   font_size;        // 像素大小
		bool       is_force_to_file; // source 参数代表文件路径，且强制以文件的方式打开，不读取进内存，当 is_buffer = true 时忽略该选项
		bool       is_buffer;        // source 参数代表二进制数据，而不是文件路径
	};

	struct IGlyphManager : public IObject
	{
		virtual float getLineHeight() = 0;
		virtual float getAscender() = 0;
		virtual float getDescender() = 0;

		virtual uint32_t getTextureCount() = 0;
		virtual ITexture2D* getTexture(uint32_t index) = 0;

		virtual bool cacheGlyph(uint32_t codepoint) = 0;
		virtual bool cacheString(StringView str) = 0;
		virtual bool flush() = 0;

		virtual bool getGlyph(uint32_t codepoint, GlyphInfo* p_ref_info, bool no_render) = 0;

		static bool create(IDevice* p_device, TrueTypeFontInfo const* p_arr_info, size_t info_count, IGlyphManager** output);
	};

	struct ITextRenderer : public IObject
	{
		virtual void setScale(Vector2F const& scale) = 0;
		virtual Vector2F getScale() = 0;
		virtual void setColor(Color4B const color) = 0;
		virtual Color4B getColor() = 0;
		virtual void setZ(float const z) = 0;
		virtual float getZ() = 0;
		virtual void setGlyphManager(IGlyphManager* p_mgr) = 0;
		virtual IGlyphManager* getGlyphManager() = 0;

		// 测量一个字符串绘制的精确包围盒大小（受到 setScale 影响）
		virtual RectF getTextBoundary(StringView str) = 0;
		// 测量一个字符串绘制的前进量（受到 setScale 影响）
		virtual Vector2F getTextAdvance(StringView str) = 0;
		// 绘制文字，y 轴朝上（受到 setScale 影响）
		virtual bool drawText(StringView str, Vector2F const& start, Vector2F* end_output) = 0;
		// 绘制文字，提供 3D 的移动向量，以便在空间中绘制（受到 setScale 影响）
		virtual bool drawTextInSpace(StringView str,
			Vector3F const& start, Vector3F const& right_vec, Vector3F const& down_vec,
			Vector3F* end_output) = 0;

		static bool create(IRenderer* p_renderer, ITextRenderer** output);
	};
}
