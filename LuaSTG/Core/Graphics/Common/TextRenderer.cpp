#include "Core/Graphics/Common/TextRenderer.hpp"
#include "utility/utf.hpp"

namespace Core::Graphics::Common {
	
	RectF TextRenderer::getTextBoundary(StringView const str)
	{
		if (!m_glyph_mgr)
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
		float const line_height = m_glyph_mgr->getLineHeight();

		char32_t code_ = 0;
		while (reader_(code_))
		{
			if (code_ == U'\n')
			{
				start_pos.x = 0.0f;
				start_pos.y -= line_height;
				continue;
			}
			if (m_glyph_mgr->getGlyph(code_, &glyph_info, true))
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
	Vector2F TextRenderer::getTextAdvance(StringView const str)
	{
		if (!m_glyph_mgr)
		{
			return Vector2F();
		}

		utf::utf8reader reader_(str.data(), str.size());

		GlyphInfo glyph_info = {};
		Vector2F start_pos;
		bool is_updated = false;
		float const line_height = m_glyph_mgr->getLineHeight();

		char32_t code_ = 0;
		while (reader_(code_))
		{
			if (code_ == U'\n')
			{
				start_pos.x = 0.0f;
				start_pos.y -= line_height;
				continue;
			}
			if (m_glyph_mgr->getGlyph(code_, &glyph_info, true))
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
	bool TextRenderer::drawText(StringView const str, Vector2F const& start, Vector2F* const end_output)
	{
		if (!m_glyph_mgr)
		{
			return false;
		}

		// 首先，缓存所有字形
		if (!m_glyph_mgr->cacheString(str))
		{
			//return false; // 找不到的就忽略
		}
		if (!m_glyph_mgr->flush())
		{
			return false;
		}

		// 绘制参数
		GlyphInfo glyph_info = {};
		Vector2F start_pos = start; // 笔触位置
		float const line_height = m_glyph_mgr->getLineHeight() * m_scale.y; // 行高

		// 比较常用的空格
		GlyphInfo space_glyph_info = {};
		if (!m_glyph_mgr->getGlyph(uint32_t(U' '), &space_glyph_info, true))
		{
			//assert(false); return false; // 没有空格就默认它所有度量值为 0
		}
		space_glyph_info.advance.x *= m_scale.x;
		space_glyph_info.advance.y *= m_scale.y;

		// 迭代绘制所有文字
		if (m_glyph_mgr->getTextureCount() <= 1)
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
				if (m_glyph_mgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
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
			for (uint32_t idx = 0; idx < m_glyph_mgr->getTextureCount(); idx += 1)
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
					if (m_glyph_mgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
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
		if (end_output)
		{
			*end_output = start_pos;
		}

		return true;
	}
	bool TextRenderer::drawTextInSpace(StringView const str,
		Vector3F const& start, Vector3F const& right_vec, Vector3F const& down_vec,
		Vector3F* const end_output)
	{
		if (!m_glyph_mgr)
		{
			assert(false); return false;
		}

		// 首先，缓存所有字形
		if (!m_glyph_mgr->cacheString(str))
		{
			//return false; // 找不到的就忽略
		}
		if (!m_glyph_mgr->flush())
		{
			return false;
		}

		// 绘制参数
		GlyphInfo glyph_info = {};
		Vector3F line_pos = start; // 行起始笔触位置
		Vector3F start_pos = start; // 笔触位置
		float const line_height = m_glyph_mgr->getLineHeight() * m_scale.y; // 行高

		// 比较常用的空格
		GlyphInfo space_glyph_info = {};
		if (!m_glyph_mgr->getGlyph(uint32_t(U' '), &space_glyph_info, true))
		{
			//assert(false); return false; // 没有空格就默认它所有度量值为 0
		}
		space_glyph_info.advance.x *= m_scale.x;
		space_glyph_info.advance.y *= m_scale.y;

		// 迭代绘制所有文字
		if (m_glyph_mgr->getTextureCount() <= 1)
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
				if (m_glyph_mgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
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
			for (uint32_t idx = 0; idx < m_glyph_mgr->getTextureCount(); idx += 1)
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
					if (m_glyph_mgr->getGlyph(code_, &glyph_info, true)) // 不要在这里 flush
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
		if (end_output)
		{
			*end_output = start_pos;
		}

		return true;
	}

	TextRenderer::TextRenderer(IRenderer* const p_renderer)
		: m_renderer(p_renderer)
		, m_scale(Vector2F(1.0f, 1.0f))
		, m_z(0.5f)
		, m_color(Color4B(0xFFFFFFFFu))
	{
	}
	TextRenderer::~TextRenderer() = default;

	bool TextRenderer::drawGlyph(GlyphInfo const& glyph_info, Vector2F const& start_pos) {
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
		auto const p_texture = m_glyph_mgr->getTexture(glyph_info.texture_index);
		if (!p_texture) {
			assert(false); return false;
		}

		// 绘图
		m_renderer->setTexture(p_texture);
		m_renderer->drawQuad(vert);

		return true;
	}
	bool TextRenderer::drawGlyphInSpace(GlyphInfo const& glyph_info, Vector3F const& start_pos, Vector3F const& right_vec, Vector3F const& down_vec) {
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
		auto const p_texture = m_glyph_mgr->getTexture(glyph_info.texture_index);
		if (!p_texture) {
			assert(false); return false;
		}

		// 绘图
		m_renderer->setTexture(p_texture);
		m_renderer->drawQuad(vert);

		return true;
	}

}
namespace Core::Graphics {
	bool ITextRenderer::create(IRenderer* const p_renderer, ITextRenderer** const output) {
		try {
			*output = new Common::TextRenderer(p_renderer);
			return true;
		} catch (...) {
			*output = nullptr;
			return false;
		}
	}
}
