#include "Core/Graphics/Sprite_D3D11.hpp"

namespace LuaSTG::Core::Graphics
{
	void Sprite_D3D11::setTextureRect(RectF const& rc)
	{
		m_rect = rc;
		Vector2U const size = m_texture->getSize();
		float const uscale = 1.0f / (float)size.x;
		float const vscale = 1.0f / (float)size.y;
		m_uv = rc;
		m_uv.a.x *= uscale;
		m_uv.a.y *= vscale;
		m_uv.b.x *= uscale;
		m_uv.b.y *= vscale;
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
		draw(pos, Vector2F(scale, scale), rotation);
	}
	void Sprite_D3D11::draw(Vector2F const& pos, Vector2F const& scale)
	{
		m_renderer->setTexture(m_texture.get());

		// 纹理坐标系 y 轴朝下，渲染坐标系 y 轴朝上

		Vector2F const scale_ = scale * m_scale;

		RectF rect = m_rect - m_center;
		rect.a.x *= scale_.x;
		rect.a.y *= -scale_.y;
		rect.b.x *= scale_.x;
		rect.b.y *= -scale_.y;

		IRenderer::DrawVertex vert[4] = {
			IRenderer::DrawVertex(pos.x + rect.a.x, pos.y + rect.a.y, m_z, m_uv.a.x, m_uv.a.y, m_color[0].u.color),
			IRenderer::DrawVertex(pos.x + rect.b.x, pos.y + rect.a.y, m_z, m_uv.b.x, m_uv.a.y, m_color[1].u.color),
			IRenderer::DrawVertex(pos.x + rect.b.x, pos.y + rect.b.y, m_z, m_uv.b.x, m_uv.b.y, m_color[2].u.color),
			IRenderer::DrawVertex(pos.x + rect.a.x, pos.y + rect.b.y, m_z, m_uv.a.x, m_uv.b.y, m_color[3].u.color),
		};

		m_renderer->drawQuad(vert);
	}
	void Sprite_D3D11::draw(Vector2F const& pos, Vector2F const& scale, float rotation)
	{
		m_renderer->setTexture(m_texture.get());

		// 纹理坐标系 y 轴朝下，渲染坐标系 y 轴朝上

		Vector2F const scale_ = scale * m_scale;

		RectF rect = m_rect - m_center;
		rect.a.x *= scale_.x;
		rect.a.y *= -scale_.y;
		rect.b.x *= scale_.x;
		rect.b.y *= -scale_.y;

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
		setTextureRect(RectF(0.0f, 0.0f, (float)size.x, (float)size.y));
		m_center = Vector2F(m_rect.b.x * 0.5f, m_rect.b.y * 0.5f);
	}
	Sprite_D3D11::~Sprite_D3D11()
	{
	}
}
