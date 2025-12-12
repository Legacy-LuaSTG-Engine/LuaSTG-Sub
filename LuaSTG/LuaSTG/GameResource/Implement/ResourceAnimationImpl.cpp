#include "GameResource/Implement/ResourceAnimationImpl.hpp"
#include "GameResource/Implement/ResourceSpriteImpl.hpp"
#include "GameResource/LegacyBlendStateHelper.hpp"
#include "GameResource/SharedSpriteRenderer.hpp"
#include "AppFrame.h"

namespace luastg
{
	ResourceAnimationImpl::ResourceAnimationImpl(
		const char* name, core::SmartReference<IResourceTexture> tex,
		float x, float y, float w, float h,
		int n, int m, int intv,
		double a, double b, bool rect)
		: ResourceBaseImpl(ResourceType::Animation, name)
		, m_Interval(intv)
		, m_HalfSizeX(a)
		, m_HalfSizeY(b)
		, m_bRectangle(rect)
		, m_is_sprite_cloned(true)
	{
		// 分割纹理
		m_sprites.reserve(m * n);
		for (int j = 0; j < m; ++j)  // 行
		{
			for (int i = 0; i < n; ++i)  // 列
			{
				core::SmartReference<core::Graphics::ISprite> p_sprite_core;
				if (!core::Graphics::ISprite::create(
					tex->GetTexture(),
					p_sprite_core.put()
				))
				{
					throw std::runtime_error("ResourceAnimationImpl::ResourceAnimationImpl");
				}
				core::RectF rc = core::RectF(
					x + w * i,
					y + h * j,
					x + w * (i + 1),
					y + h * (j + 1)
				);
				p_sprite_core->setTextureRect(rc);
				p_sprite_core->setTextureCenter(core::Vector2F(
					(rc.a.x + rc.b.x) * 0.5f,
					(rc.a.y + rc.b.y) * 0.5f
				));
				core::SmartReference<IResourceSprite> p_sprite;
				p_sprite.attach(new ResourceSpriteImpl("", p_sprite_core.get(), a, b, rect));
				m_sprites.emplace_back(p_sprite);
			}
		}
	}
	ResourceAnimationImpl::ResourceAnimationImpl(
		const char* name,
		std::vector<core::SmartReference<IResourceSprite>> const& sprite_list,
		int intv,
		double a, double b, bool rect)
		: ResourceBaseImpl(ResourceType::Animation, name)
		, m_Interval(intv)
		, m_HalfSizeX(a)
		, m_HalfSizeY(b)
		, m_bRectangle(rect)
		, m_is_sprite_cloned(false)
	{
		m_sprites.reserve(sprite_list.size());
		for (auto v : sprite_list)
		{
			m_sprites.push_back(v);
		}
	}

	IResourceSprite* ResourceAnimationImpl::GetSprite(uint32_t index)
	{
		if (index >= GetCount())
		{
			assert(false); return nullptr;
		}
		return m_sprites[index].get();
	}
	uint32_t ResourceAnimationImpl::GetSpriteIndexByTimer(int ani_timer)
	{
		return ((uint32_t)ani_timer / m_Interval) % GetCount();
	}
	IResourceSprite* ResourceAnimationImpl::GetSpriteByTimer(int ani_timer)
	{
		return m_sprites[GetSpriteIndexByTimer(ani_timer)].get();
	}
	void ResourceAnimationImpl::Render(int timer, float x, float y, float rot, float hscale, float vscale, float z)
	{
		const auto sprite = GetSpriteByTimer(timer)->GetSprite();
		const auto renderer = SharedSpriteRenderer::getInstance();
		const auto blend = luastg::translateLegacyBlendState(m_BlendMode);

		renderer->setSprite(sprite);
		renderer->setTransform(core::Vector2F(x, y), core::Vector2F(hscale, vscale), rot);
		renderer->setZ(z);
		renderer->setLegacyBlendState(blend.vertex_color_blend_state, blend.blend_state);
		renderer->setColor(m_vertex_color[0], m_vertex_color[1], m_vertex_color[2], m_vertex_color[3]);

		renderer->draw(LAPP.GetRenderer2D());

		renderer->setSprite(nullptr);
	}
	void ResourceAnimationImpl::Render(int timer, float x, float y, float rot, float hscale, float vscale, BlendMode blend_, core::Color4B color, float z)
	{
		const auto sprite = GetSpriteByTimer(timer)->GetSprite();
		const auto renderer = SharedSpriteRenderer::getInstance();
		const auto blend = luastg::translateLegacyBlendState(blend_);

		renderer->setSprite(sprite);
		renderer->setTransform(core::Vector2F(x, y), core::Vector2F(hscale, vscale), rot);
		renderer->setZ(z);
		renderer->setLegacyBlendState(blend.vertex_color_blend_state, blend.blend_state);
		renderer->setColor(color);

		renderer->draw(LAPP.GetRenderer2D());

		renderer->setSprite(nullptr);
	}
}
