#include "Core/Graphics/Sprite_D3D11.hpp"

namespace LuaSTG::Core::Graphics
{
	void Sprite_D3D11::updateRect()
	{
		// 纹理坐标系需要归一化

		Vector2U const size = m_texture->getSize();
		float const uscale = 1.0f / (float)size.x;
		float const vscale = 1.0f / (float)size.y;
		m_uv = m_rect;
		m_uv.a.x *= uscale;
		m_uv.a.y *= vscale;
		m_uv.b.x *= uscale;
		m_uv.b.y *= vscale;

		// 纹理坐标系 y 轴朝下，渲染坐标系 y 轴朝上

		m_pos_rc = m_rect - m_center;
		m_pos_rc.a.x *= m_scale;
		m_pos_rc.a.y *= -m_scale;
		m_pos_rc.b.x *= m_scale;
		m_pos_rc.b.y *= -m_scale;
	}

	void Sprite_D3D11::setTextureRect(RectF const& rc)
	{
		m_rect = rc;
		updateRect();
	}
	void Sprite_D3D11::setTextureCenter(Vector2F const& pt)
	{
		m_center = pt;
		updateRect();
	}
	void Sprite_D3D11::setUnitsPerPixel(float v)
	{
		m_scale = v;
		updateRect();
	}

	void Sprite_D3D11::draw(RectF const& rc)
	{
		m_renderer->setTexture(m_texture.get());

		IRenderer::DrawVertex const vert[4] = {
			IRenderer::DrawVertex(rc.a.x, rc.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].u.color),
			IRenderer::DrawVertex(rc.b.x, rc.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].u.color),
			IRenderer::DrawVertex(rc.b.x, rc.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].u.color),
			IRenderer::DrawVertex(rc.a.x, rc.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].u.color),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite_D3D11::draw(Vector3F const& p1, Vector3F const& p2, Vector3F const& p3, Vector3F const& p4)
	{
		m_renderer->setTexture(m_texture.get());

		IRenderer::DrawVertex const vert[4] = {
			IRenderer::DrawVertex(p1.x, p1.y, p1.z, m_uv.a.x, m_uv.a.y, m_color[0].u.color),
			IRenderer::DrawVertex(p2.x, p2.y, p2.z, m_uv.b.x, m_uv.a.y, m_color[1].u.color),
			IRenderer::DrawVertex(p3.x, p3.y, p3.z, m_uv.b.x, m_uv.b.y, m_color[2].u.color),
			IRenderer::DrawVertex(p4.x, p4.y, p4.z, m_uv.a.x, m_uv.b.y, m_color[3].u.color),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite_D3D11::draw(Vector2F const& pos)
	{
		draw(pos, Vector2F(1.0f, 1.0f));
	}
	void Sprite_D3D11::draw(Vector2F const& pos, float scale)
	{
		draw(pos, Vector2F(scale, scale));
	}
	void Sprite_D3D11::draw(Vector2F const& pos, float scale, float rotation)
	{
		if (std::abs(rotation) < std::numeric_limits<float>::min())
		{
			draw(pos, Vector2F(scale, scale));
			return;
		}

		draw(pos, Vector2F(scale, scale), rotation);
	}
	void Sprite_D3D11::draw(Vector2F const& pos, Vector2F const& scale)
	{
		m_renderer->setTexture(m_texture.get());

		RectF const rect = RectF(
			m_pos_rc.a.x *= scale.x,
			m_pos_rc.a.y *= scale.y,
			m_pos_rc.b.x *= scale.x,
			m_pos_rc.b.y *= scale.y
		);
		
		IRenderer::DrawVertex const vert[4] = {
			IRenderer::DrawVertex(pos.x + rect.a.x, pos.y + rect.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].u.color),
			IRenderer::DrawVertex(pos.x + rect.b.x, pos.y + rect.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].u.color),
			IRenderer::DrawVertex(pos.x + rect.b.x, pos.y + rect.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].u.color),
			IRenderer::DrawVertex(pos.x + rect.a.x, pos.y + rect.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].u.color),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite_D3D11::draw(Vector2F const& pos, Vector2F const& scale, float rotation)
	{
		if (std::abs(rotation) < std::numeric_limits<float>::min())
		{
			draw(pos, scale);
			return;
		}

		m_renderer->setTexture(m_texture.get());

		RectF const rect = RectF(
			m_pos_rc.a.x *= scale.x,
			m_pos_rc.a.y *= scale.y,
			m_pos_rc.b.x *= scale.x,
			m_pos_rc.b.y *= scale.y
		);

		IRenderer::DrawVertex vert[4] = {
			IRenderer::DrawVertex(rect.a.x, rect.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].u.color),
			IRenderer::DrawVertex(rect.b.x, rect.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].u.color),
			IRenderer::DrawVertex(rect.b.x, rect.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].u.color),
			IRenderer::DrawVertex(rect.a.x, rect.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].u.color),
		};

		float const sinv = std::sinf(rotation);
		float const cosv = std::cosf(rotation);

#define rotate_xy(UNIT) \
		{\
			float const tx = vert[UNIT].x * cosv - vert[UNIT].y * sinv;\
			float const ty = vert[UNIT].x * sinv + vert[UNIT].y * cosv;\
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

	bool Sprite_D3D11::clone(ISprite** pp_sprite)
	{
		try
		{
			*pp_sprite = new Sprite_D3D11(m_renderer.get(), m_texture.get());
			Sprite_D3D11* right = static_cast<Sprite_D3D11*>(*pp_sprite);
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
			return true;
		}
		catch (...)
		{
			*pp_sprite = nullptr;
			return false;
		}
	}

	Sprite_D3D11::Sprite_D3D11(IRenderer* p_renderer, ITexture2D* p_texture)
		: m_renderer(p_renderer)
		, m_texture(p_texture)
		, m_z(0.5f)
		, m_scale(1.0f)
	{
		m_color[0] = Color4B(0xFFFFFFFFu);
		m_color[1] = Color4B(0xFFFFFFFFu);
		m_color[2] = Color4B(0xFFFFFFFFu);
		m_color[3] = Color4B(0xFFFFFFFFu);
		Vector2U const size = m_texture->getSize();
		m_rect = RectF(0.0f, 0.0f, (float)size.x, (float)size.y);
		m_center = Vector2F(m_rect.b.x * 0.5f, m_rect.b.y * 0.5f);
		updateRect();
	}
	Sprite_D3D11::~Sprite_D3D11()
	{
	}

	bool ISprite::create(IRenderer* p_renderer, ITexture2D* p_texture, ISprite** pp_sprite)
	{
		try
		{
			*pp_sprite = new Sprite_D3D11(p_renderer, p_texture);
			return true;
		}
		catch (...)
		{
			*pp_sprite = nullptr;
			return false;
		}
	}
}
