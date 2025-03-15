#include "Core/Graphics/Common/SpriteRenderer.hpp"
#include <cassert>

namespace Core::Graphics::Common {
	void SpriteRenderer::setTransform(RectF const& rect) {
		m_vertex[0].x = rect.a.x;
		m_vertex[0].y = rect.a.y;
		m_vertex[1].x = rect.b.x;
		m_vertex[1].y = rect.a.y;
		m_vertex[2].x = rect.b.x;
		m_vertex[2].y = rect.b.y;
		m_vertex[3].x = rect.a.x;
		m_vertex[3].y = rect.b.y;
	}
	void SpriteRenderer::setTransform(Vector2F const& p1, Vector2F const& p2, Vector2F const& p3, Vector2F const& p4) {
		m_vertex[0].x = p1.x;
		m_vertex[0].y = p1.y;
		m_vertex[1].x = p2.x;
		m_vertex[1].y = p2.y;
		m_vertex[2].x = p3.x;
		m_vertex[2].y = p3.y;
		m_vertex[3].x = p4.x;
		m_vertex[3].y = p4.y;
	}
	void SpriteRenderer::setTransform(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) {
		m_vertex[0].x = p1.x;
		m_vertex[0].y = p1.y;
		m_vertex[0].z = p1.z;
		m_vertex[1].x = p2.x;
		m_vertex[1].y = p2.y;
		m_vertex[1].z = p2.z;
		m_vertex[2].x = p3.x;
		m_vertex[2].y = p3.y;
		m_vertex[2].z = p3.z;
		m_vertex[3].x = p4.x;
		m_vertex[3].y = p4.y;
		m_vertex[3].z = p4.z;
	}
	void SpriteRenderer::setTransform(Vector2F const& position, Vector2F const& scale) {
		auto const rect0 = m_sprite->getTextureRect() - m_sprite->getTextureCenter();
		auto const scale0 = Vector2F(m_sprite->getUnitsPerPixel(), -m_sprite->getUnitsPerPixel()); // Y 轴朝上
		auto const rect = RectF(rect0.a * scale0 * scale, rect0.b * scale0 * scale);
		m_vertex[0].x = position.x + rect.a.x;
		m_vertex[0].y = position.y + rect.a.y;
		m_vertex[1].x = position.x + rect.b.x;
		m_vertex[1].y = position.y + rect.a.y;
		m_vertex[2].x = position.x + rect.b.x;
		m_vertex[2].y = position.y + rect.b.y;
		m_vertex[3].x = position.x + rect.a.x;
		m_vertex[3].y = position.y + rect.b.y;
	}
	void SpriteRenderer::setTransform(Vector2F const& position, Vector2F const& scale, float const rotation) {
		if (std::abs(rotation) < std::numeric_limits<float>::min()) {
			setTransform(position, scale);
			return;
		}
		auto const rect0 = m_sprite->getTextureRect() - m_sprite->getTextureCenter();
		auto const scale0 = Vector2F(m_sprite->getUnitsPerPixel(), -m_sprite->getUnitsPerPixel()); // Y 轴朝上
		auto const rect = RectF(rect0.a * scale0 * scale, rect0.b * scale0 * scale);
		m_vertex[0].x = rect.a.x;
		m_vertex[0].y = rect.a.y;
		m_vertex[1].x = rect.b.x;
		m_vertex[1].y = rect.a.y;
		m_vertex[2].x = rect.b.x;
		m_vertex[2].y = rect.b.y;
		m_vertex[3].x = rect.a.x;
		m_vertex[3].y = rect.b.y;
		auto const sin_v = std::sinf(rotation);
		auto const cos_v = std::cosf(rotation);
	#define ROTATE_X_Y(UNIT) \
		{\
			auto const x = m_vertex[UNIT].x * cos_v - m_vertex[UNIT].y * sin_v;\
			auto const y = m_vertex[UNIT].x * sin_v + m_vertex[UNIT].y * cos_v;\
			m_vertex[UNIT].x = x;\
			m_vertex[UNIT].y = y;\
		}
		ROTATE_X_Y(0);
		ROTATE_X_Y(1);
		ROTATE_X_Y(2);
		ROTATE_X_Y(3);
	#undef ROTATE_X_Y
		m_vertex[0].x += position.x;
		m_vertex[0].y += position.y;
		m_vertex[1].x += position.x;
		m_vertex[1].y += position.y;
		m_vertex[2].x += position.x;
		m_vertex[2].y += position.y;
		m_vertex[3].x += position.x;
		m_vertex[3].y += position.y;
	}
	void SpriteRenderer::setSprite(ISprite* const sprite) {
		assert(sprite);
		m_sprite = sprite;
		if (m_sprite) {
			auto const rect = m_sprite->getTextureRect();
			auto const size = m_sprite->getTexture()->getSize();
			auto const u_scale = 1.0f / static_cast<float>(size.x);
			auto const v_scale = 1.0f / static_cast<float>(size.y);
			auto uv_rect = rect;
			uv_rect.a.x *= u_scale;
			uv_rect.a.y *= v_scale;
			uv_rect.b.x *= u_scale;
			uv_rect.b.y *= v_scale;
			m_vertex[0].u = uv_rect.a.x;
			m_vertex[0].v = uv_rect.a.y;
			m_vertex[1].u = uv_rect.b.x;
			m_vertex[1].v = uv_rect.a.y;
			m_vertex[2].u = uv_rect.b.x;
			m_vertex[2].v = uv_rect.b.y;
			m_vertex[3].u = uv_rect.a.x;
			m_vertex[3].v = uv_rect.b.y;
		}
	}
	void SpriteRenderer::setColor(Color4B const color) {
		for (auto& vertex : m_vertex) {
			vertex.color = color.color();
		}
	}
	void SpriteRenderer::setColor(Color4B const c1, Color4B const c2, Color4B const c3, Color4B const c4) {
		m_vertex[0].color = c1.color();
		m_vertex[1].color = c2.color();
		m_vertex[2].color = c3.color();
		m_vertex[3].color = c4.color();
	}
	void SpriteRenderer::setZ(float const z) {
		for (auto& vertex : m_vertex) {
			vertex.z = z;
		}
	}
	void SpriteRenderer::setLegacyBlendState(IRenderer::VertexColorBlendState const vertex_color_blend_state, IRenderer::BlendState const blend_state) {
		m_vertex_color_blend_state = vertex_color_blend_state;
		m_blend_state = blend_state;
	}
	void SpriteRenderer::draw(IRenderer* const renderer) {
		assert(renderer);
		renderer->setVertexColorBlendState(m_vertex_color_blend_state);
		renderer->setBlendState(m_blend_state);
		renderer->setTexture(m_sprite->getTexture());
		renderer->drawQuad(m_vertex);
	}

	SpriteRenderer::SpriteRenderer() {
		setColor(Color4B(0xffffffffu));
	}
	SpriteRenderer::~SpriteRenderer() = default;
}
namespace Core::Graphics {
	bool ISpriteRenderer::create(ISpriteRenderer** const output) {
		if (output == nullptr) {
			assert(false);
			return false;
		}
		*output = new Common::SpriteRenderer;
		return true;
	}
}
