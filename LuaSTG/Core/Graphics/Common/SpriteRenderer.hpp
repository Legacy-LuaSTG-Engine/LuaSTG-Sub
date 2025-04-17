#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace core::Graphics::Common {
	class SpriteRenderer final
		: public implement::ReferenceCounted<ISpriteRenderer>
	{
	public:
		// ISpriteRenderer

		void setTransform(RectF const& rect) override;
		void setTransform(Vector2F const& p1, Vector2F const& p2, Vector2F const& p3, Vector2F const& p4) override;
		void setTransform(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) override;
		void setTransform(Vector2F const& position, Vector2F const& scale);
		void setTransform(Vector2F const& position, Vector2F const& scale, float rotation) override;
		void setSprite(ISprite* sprite) override;
		void setColor(Color4B color) override;
		void setColor(Color4B c1, Color4B c2, Color4B c3, Color4B c4) override;
		void setZ(float z) override;
		void setLegacyBlendState(IRenderer::VertexColorBlendState vertex_color_blend_state, IRenderer::BlendState blend_state) override;
		void draw(IRenderer* renderer) override;

		// SpriteRenderer

		SpriteRenderer();
		SpriteRenderer(SpriteRenderer const&) = delete;
		SpriteRenderer(SpriteRenderer&&) = delete;
		~SpriteRenderer();

		SpriteRenderer& operator=(SpriteRenderer const&) = delete;
		SpriteRenderer& operator=(SpriteRenderer&&) = delete;

	private:
		ScopeObject<ISprite> m_sprite;
		IRenderer::DrawVertex m_vertex[4]{};
		IRenderer::VertexColorBlendState m_vertex_color_blend_state{ IRenderer::VertexColorBlendState::Mul };
		IRenderer::BlendState m_blend_state{ IRenderer::BlendState::Alpha };
	};
}
