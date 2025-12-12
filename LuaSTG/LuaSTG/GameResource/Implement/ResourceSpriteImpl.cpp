#include "GameResource/Implement/ResourceSpriteImpl.hpp"
#include "AppFrame.h"
#include "GameResource/LegacyBlendStateHelper.hpp"

namespace {
	class Instance {
	public:
		Instance() {
			std::ignore = core::Graphics::ISpriteRenderer::create(m_renderer.put());
		}

		core::Graphics::ISpriteRenderer* get() { return m_renderer.get(); }

	private:
		core::SmartReference<core::Graphics::ISpriteRenderer> m_renderer;
	};

	Instance s_renderer;
}

namespace luastg
{
	ResourceSpriteImpl::ResourceSpriteImpl(const char* name, core::Graphics::ISprite* sprite, double hx, double hy, bool rect)
		: ResourceBaseImpl(ResourceType::Sprite, name)
		, m_sprite(sprite)
		, m_HalfSizeX(hx)
		, m_HalfSizeY(hy)
		, m_bRectangle(rect)
	{
	}

	void ResourceSpriteImpl::RenderRect(float l, float r, float b, float t, float z)
	{
		const auto renderer = s_renderer.get();
		const auto blend = luastg::translateLegacyBlendState(m_BlendMode);

		renderer->setSprite(m_sprite.get());
		renderer->setTransform(core::RectF(l, t, r, b));
		renderer->setZ(z);
		renderer->setLegacyBlendState(blend.vertex_color_blend_state, blend.blend_state);
		renderer->setColor(m_color[0], m_color[1], m_color[2], m_color[3]);

		renderer->draw(LAPP.GetRenderer2D());

		renderer->setSprite(nullptr);
	}
	void ResourceSpriteImpl::Render(float x, float y, float rot, float hscale, float vscale, float z)
	{
		const auto renderer = s_renderer.get();
		const auto blend = luastg::translateLegacyBlendState(m_BlendMode);

		renderer->setSprite(m_sprite.get());
		renderer->setTransform(core::Vector2F(x, y), core::Vector2F(hscale, vscale), rot);
		renderer->setZ(z);
		renderer->setLegacyBlendState(blend.vertex_color_blend_state, blend.blend_state);
		renderer->setColor(m_color[0], m_color[1], m_color[2], m_color[3]);

		renderer->draw(LAPP.GetRenderer2D());

		renderer->setSprite(nullptr);
	}
	void ResourceSpriteImpl::Render(float x, float y, float rot, float hscale, float vscale, BlendMode blend_, core::Color4B color, float z)
	{
		const auto renderer = s_renderer.get();
		const auto blend = luastg::translateLegacyBlendState(blend_);

		renderer->setSprite(m_sprite.get());
		renderer->setTransform(core::Vector2F(x, y), core::Vector2F(hscale, vscale), rot);
		renderer->setZ(z);
		renderer->setLegacyBlendState(blend.vertex_color_blend_state, blend.blend_state);
		renderer->setColor(color);

		renderer->draw(LAPP.GetRenderer2D());

		renderer->setSprite(nullptr);
	}
	void ResourceSpriteImpl::Render4V(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4)
	{
		const auto renderer = s_renderer.get();
		const auto blend = luastg::translateLegacyBlendState(m_BlendMode);

		renderer->setSprite(m_sprite.get());
		renderer->setTransform(
			core::Vector3F(x1, y1, z1),
			core::Vector3F(x2, y2, z2),
			core::Vector3F(x3, y3, z3),
			core::Vector3F(x4, y4, z4)
		);
		renderer->setLegacyBlendState(blend.vertex_color_blend_state, blend.blend_state);
		renderer->setColor(m_color[0], m_color[1], m_color[2], m_color[3]);

		renderer->draw(LAPP.GetRenderer2D());

		renderer->setSprite(nullptr);
	}
}
