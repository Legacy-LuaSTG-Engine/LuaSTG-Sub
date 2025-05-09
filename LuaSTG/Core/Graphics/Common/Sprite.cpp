#include "Core/Graphics/Common/Sprite.hpp"

namespace core::Graphics::Common {
	void Sprite::updateRect() {
		// 纹理坐标系需要归一化

		Vector2U const size = m_texture->getSize();
		float const u_scale = 1.0f / static_cast<float>(size.x);
		float const v_scale = 1.0f / static_cast<float>(size.y);
		m_uv = m_rect;
		m_uv.a.x *= u_scale;
		m_uv.a.y *= v_scale;
		m_uv.b.x *= u_scale;
		m_uv.b.y *= v_scale;

		// 纹理坐标系 y 轴朝下，渲染坐标系 y 轴朝上

		m_pos_rc = m_rect - m_center;
		m_pos_rc.a.x *= m_scale;
		m_pos_rc.a.y *= -m_scale;
		m_pos_rc.b.x *= m_scale;
		m_pos_rc.b.y *= -m_scale;
	}

	void Sprite::setTextureRect(RectF const& rc) {
		m_rect = rc;
		updateRect();
	}
	void Sprite::setTextureCenter(Vector2F const& pt) {
		m_center = pt;
		updateRect();
	}
	void Sprite::setUnitsPerPixel(float const v) {
		m_scale = v;
		updateRect();
	}

	void Sprite::draw(RectF const& rc) {
		m_renderer->setTexture(m_texture.get());

		IRenderer::DrawVertex const vert[4] = {
			IRenderer::DrawVertex(rc.a.x, rc.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].color()),
			IRenderer::DrawVertex(rc.b.x, rc.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].color()),
			IRenderer::DrawVertex(rc.b.x, rc.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].color()),
			IRenderer::DrawVertex(rc.a.x, rc.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].color()),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite::draw(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4) {
		m_renderer->setTexture(m_texture.get());

		IRenderer::DrawVertex const vert[4] = {
			IRenderer::DrawVertex(p1.x, p1.y, p1.z, m_uv.a.x, m_uv.a.y, m_color[0].color()),
			IRenderer::DrawVertex(p2.x, p2.y, p2.z, m_uv.b.x, m_uv.a.y, m_color[1].color()),
			IRenderer::DrawVertex(p3.x, p3.y, p3.z, m_uv.b.x, m_uv.b.y, m_color[2].color()),
			IRenderer::DrawVertex(p4.x, p4.y, p4.z, m_uv.a.x, m_uv.b.y, m_color[3].color()),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite::draw(Vector2F const& pos) {
		draw(pos, Vector2F(1.0f, 1.0f));
	}
	void Sprite::draw(Vector2F const& pos, float const scale) {
		draw(pos, Vector2F(scale, scale));
	}
	void Sprite::draw(Vector2F const& pos, float const scale, float const rotation) {
		if (std::abs(rotation) < std::numeric_limits<float>::min()) {
			draw(pos, Vector2F(scale, scale));
			return;
		}

		draw(pos, Vector2F(scale, scale), rotation);
	}
	void Sprite::draw(Vector2F const& pos, Vector2F const& scale) {
		m_renderer->setTexture(m_texture.get());

		auto const rect = RectF(
			m_pos_rc.a.x * scale.x,
			m_pos_rc.a.y * scale.y,
			m_pos_rc.b.x * scale.x,
			m_pos_rc.b.y * scale.y
		);

		IRenderer::DrawVertex const vert[4] = {
			IRenderer::DrawVertex(pos.x + rect.a.x, pos.y + rect.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].color()),
			IRenderer::DrawVertex(pos.x + rect.b.x, pos.y + rect.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].color()),
			IRenderer::DrawVertex(pos.x + rect.b.x, pos.y + rect.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].color()),
			IRenderer::DrawVertex(pos.x + rect.a.x, pos.y + rect.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].color()),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite::draw(Vector2F const& pos, Vector2F const& scale, float const rotation) {
		if (std::abs(rotation) < std::numeric_limits<float>::min()) {
			draw(pos, scale);
			return;
		}

		m_renderer->setTexture(m_texture.get());

		auto const rect = RectF(
			m_pos_rc.a.x * scale.x,
			m_pos_rc.a.y * scale.y,
			m_pos_rc.b.x * scale.x,
			m_pos_rc.b.y * scale.y
		);

		IRenderer::DrawVertex vert[4] = {
			IRenderer::DrawVertex(rect.a.x, rect.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].color()),
			IRenderer::DrawVertex(rect.b.x, rect.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].color()),
			IRenderer::DrawVertex(rect.b.x, rect.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].color()),
			IRenderer::DrawVertex(rect.a.x, rect.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].color()),
		};

		float const sin_v = std::sinf(rotation);
		float const cos_v = std::cosf(rotation);

	#define rotate_xy(UNIT) \
		{\
			float const tx = vert[UNIT].x * cos_v - vert[UNIT].y * sin_v;\
			float const ty = vert[UNIT].x * sin_v + vert[UNIT].y * cos_v;\
			vert[UNIT].x = tx;\
			vert[UNIT].y = ty;\
		}

		rotate_xy(0);
		rotate_xy(1);
		rotate_xy(2);
		rotate_xy(3);

		vert[0].x += pos.x; vert[0].y += pos.y;
		vert[1].x += pos.x; vert[1].y += pos.y;
		vert[2].x += pos.x; vert[2].y += pos.y;
		vert[3].x += pos.x; vert[3].y += pos.y;

		m_renderer->drawQuad(vert);
	}

	bool Sprite::clone(ISprite** const pp_sprite) {
		auto const right = new Sprite(m_renderer.get(), m_texture.get());
		right->m_rect = m_rect;
		right->m_pos_rc = m_pos_rc;
		right->m_uv = m_uv;
		right->m_center = m_center;
		right->m_z = m_z;
		right->m_scale = m_scale;
		right->m_color[0] = m_color[0];
		right->m_color[1] = m_color[1];
		right->m_color[2] = m_color[2];
		right->m_color[3] = m_color[3];
		*pp_sprite = right;
		return true;
	}

	Sprite::Sprite(IRenderer* const p_renderer, ITexture2D* const p_texture)
		: m_renderer(p_renderer)
		, m_texture(p_texture)
		, m_z(0.5f)
		, m_scale(1.0f) {
		m_color[0] = Color4B(0xFFFFFFFFu);
		m_color[1] = Color4B(0xFFFFFFFFu);
		m_color[2] = Color4B(0xFFFFFFFFu);
		m_color[3] = Color4B(0xFFFFFFFFu);
		Vector2U const size = m_texture->getSize();
		m_rect = RectF(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y));
		m_center = Vector2F(m_rect.b.x * 0.5f, m_rect.b.y * 0.5f);
		updateRect();
	}
	Sprite::~Sprite() = default;
}
namespace core::Graphics {
	bool ISprite::create(IRenderer* const p_renderer, ITexture2D* const p_texture, ISprite** const pp_sprite) {
		*pp_sprite = new Common::Sprite(p_renderer, p_texture);
		return true;
	}
}
