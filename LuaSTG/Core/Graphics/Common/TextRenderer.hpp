#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Font.hpp"

namespace Core::Graphics::Common
{
	class TextRenderer final
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
		TextRenderer(IRenderer* p_renderer);
		~TextRenderer();
	};
}
