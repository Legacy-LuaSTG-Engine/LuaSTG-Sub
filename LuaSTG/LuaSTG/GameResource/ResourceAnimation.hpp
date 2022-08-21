#pragma once
#include "GameResource/ResourceBase.hpp"
#include "GameResource/ResourceTexture.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTGPlus
{
	// 动画资源
	class ResAnimation : public Resource
	{
	private:
		std::vector<Core::ScopeObject<Core::Graphics::ISprite>> m_sprites;
		uint32_t m_Interval = 1;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		size_t GetCount() const noexcept { return m_sprites.size(); }
		Core::Graphics::ISprite* GetSprite(uint32_t index);
		uint32_t GetSpriteIndexByTimer(int ani_timer);
		Core::Graphics::ISprite* GetSpriteByTimer(int ani_timer);
		uint32_t GetInterval() const noexcept { return m_Interval; }
		BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
		void SetBlendMode(BlendMode m) noexcept { m_BlendMode = m; }
		double GetHalfSizeX() const noexcept { return m_HalfSizeX; }
		double GetHalfSizeY() const noexcept { return m_HalfSizeY; }
		bool IsRectangle() const noexcept { return m_bRectangle; }
	public:
		ResAnimation(const char* name, fcyRefPointer<ResTexture> tex,
			float x, float y, float w, float h,
			int n, int m, int intv,
			double a, double b, bool rect = false);
		~ResAnimation();
	};
}
