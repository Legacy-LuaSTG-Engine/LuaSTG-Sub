#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/Font.hpp"

namespace core::Graphics::Common
{
	class TextRenderer final
		: public implement::ReferenceCounted<ITextRenderer>
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
		bool drawText(StringView str, Vector2F const& start, Vector2F* end_output) override;
		bool drawTextInSpace(
			StringView str,
			Vector3F const& start, Vector3F const& right_vec, Vector3F const& down_vec,
			Vector3F* end_output) override;

		// TextRenderer

		explicit TextRenderer(IRenderer* p_renderer);
		TextRenderer(TextRenderer const&) = delete;
		TextRenderer(TextRenderer&&) = delete;
		~TextRenderer();

		TextRenderer& operator=(TextRenderer const&) = delete;
		TextRenderer& operator=(TextRenderer&&) = delete;

	private:
		bool drawGlyph(GlyphInfo const& glyph_info, Vector2F const& start_pos);
		bool drawGlyphInSpace(GlyphInfo const& glyph_info, Vector3F const& start_pos, Vector3F const& right_vec, Vector3F const& down_vec);

		SmartReference<IRenderer> m_renderer;
		SmartReference<IGlyphManager> m_glyph_mgr;
		Vector2F m_scale;
		float m_z{ 0.0f };
		Color4B m_color;
	};
}
