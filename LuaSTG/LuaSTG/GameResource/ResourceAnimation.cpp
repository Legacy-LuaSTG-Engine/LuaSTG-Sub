#include "GameResource/ResourceAnimation.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	ResAnimation::ResAnimation(
		const char* name, fcyRefPointer<ResTexture> tex,
		float x, float y, float w, float h,
		int n, int m, int intv,
		double a, double b, bool rect
	)
		: Resource(ResourceType::Animation, name)
		, m_Interval(intv)
		, m_HalfSizeX(a)
		, m_HalfSizeY(b)
		, m_bRectangle(rect)
	{
		// 分割纹理
		m_sprites.reserve(m * n);
		for (int j = 0; j < m; ++j)  // 行
		{
			for (int i = 0; i < n; ++i)  // 列
			{
				Core::ScopeObject<Core::Graphics::ISprite> p_sprite;
				if (!Core::Graphics::ISprite::create(
					LAPP.GetAppModel()->getRenderer(),
					tex->GetTexture(),
					~p_sprite
				))
				{
					throw std::runtime_error("ResAnimation::ResAnimation");
				}
				Core::RectF rc = Core::RectF(
					x + w * i,
					y + h * j,
					x + w * (i + 1),
					y + h * (j + 1)
				);
				p_sprite->setTextureRect(rc);
				p_sprite->setTextureCenter(Core::Vector2F(
					(rc.a.x + rc.b.x) * 0.5f,
					(rc.a.y + rc.b.y) * 0.5f
				));
				m_sprites.emplace_back(p_sprite);
			}
		}
	}
	ResAnimation::~ResAnimation() {}

	Core::Graphics::ISprite* ResAnimation::GetSprite(uint32_t index)
	{
		if (index >= GetCount())
		{
			assert(false); return nullptr;
		}
		return m_sprites[index].get();
	}
	uint32_t ResAnimation::GetSpriteIndexByTimer(int ani_timer)
	{
		return ((uint32_t)ani_timer / m_Interval) % GetCount();
	}
	Core::Graphics::ISprite* ResAnimation::GetSpriteByTimer(int ani_timer)
	{
		return m_sprites[GetSpriteIndexByTimer(ani_timer)].get();
	}
}
