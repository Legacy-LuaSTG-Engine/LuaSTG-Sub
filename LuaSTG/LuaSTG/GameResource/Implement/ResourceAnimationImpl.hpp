#pragma once
#include "GameResource/ResourceTexture.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceAnimation.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace LuaSTGPlus
{
	class ResourceAnimationImpl : public ResourceBaseImpl<IResourceAnimation>
	{
	private:
		std::vector<Core::ScopeObject<IResourceSprite>> m_sprites;
		uint32_t m_Interval = 1;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
		// 图片精灵是自己创建的
		bool m_is_sprite_cloned = false;
	public:
		size_t GetCount() { return m_sprites.size(); }
		IResourceSprite* GetSprite(uint32_t index);
		uint32_t GetSpriteIndexByTimer(int ani_timer);
		IResourceSprite* GetSpriteByTimer(int ani_timer);
		uint32_t GetInterval() { return m_Interval; }
		BlendMode GetBlendMode() { return m_BlendMode; }
		void SetBlendMode(BlendMode m) { m_BlendMode = m; }
		double GetHalfSizeX() { return m_HalfSizeX; }
		double GetHalfSizeY() { return m_HalfSizeY; }
		bool IsRectangle() { return m_bRectangle; }
		bool IsSpriteCloned() { return m_is_sprite_cloned; }
	public:
		ResourceAnimationImpl(const char* name, Core::ScopeObject<IResourceTexture> tex,
			float x, float y, float w, float h,
			int n, int m, int intv,
			double a, double b, bool rect = false);
		ResourceAnimationImpl(const char* name,
			std::vector<Core::ScopeObject<IResourceSprite>> const& sprite_list,
			int intv,
			double a, double b, bool rect = false);
	};
}
