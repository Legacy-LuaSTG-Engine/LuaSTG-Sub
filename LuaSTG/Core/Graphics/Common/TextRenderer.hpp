#pragma once
#include "Core/Object.hpp"
#include "Core/Graphics/Font.hpp"

namespace Core::Graphics::Common
{
	class TextRenderer final
		: public Object<ITextRenderer>
	{
	public:
		// ITextRenderer

		void setScale(Vector2F const& scale) override { m_scale = scale; }
		Vector2F getScale() override { return m_scale; }
		void setColor(Color4B const color) override { m_color = color; }
		Color4B getColor() override { return m_color; }
		void setZ(float const z) override { m_z = z; }
		float getZ() override { return m_z; }
		void setGlyphManager(IGlyphManager* p_mgr) override { m_glyph_mgr = p_mgr; }
		IGlyphManager* getGlyphManager() override { return m_glyph_mgr.get(); }

		RectF getTextBoundary(StringView str) override;
		Vector2F getTextAdvance(StringView str) override;
		bool drawText(StringView str, Vector2F const& start, Vector2F* endout) override;
		bool drawTextInSpace(
			StringView str,
			Vector3F const& start, Vector3F const& right_vec, Vector3F const& down_vec,
			Vector3F* endout) override;

		// TextRenderer

		TextRenderer(IRenderer* p_renderer);
		~TextRenderer();

	private:
		bool drawGlyph(GlyphInfo const& glyph_info, Vector2F const& start_pos);
		bool drawGlyphInSpace(GlyphInfo const& glyph_info, Vector3F const& start_pos, Vector3F const& right_vec, Vector3F const& down_vec);

		ScopeObject<IRenderer> m_renderer;
		ScopeObject<IGlyphManager> m_glyph_mgr;
		Vector2F m_scale;
		float m_z{ 0.0f };
		Color4B m_color;
	};
}
