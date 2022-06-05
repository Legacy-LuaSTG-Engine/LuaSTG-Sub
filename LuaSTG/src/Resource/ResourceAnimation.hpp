#pragma once
#include "ResourceBase.hpp"
#include "ResourceTexture.hpp"
#include "Core/Graphics/Sprite.hpp"
#include <vector>

namespace LuaSTGPlus
{
	// 动画资源
	class ResAnimation : public Resource
	{
	private:
		std::vector<LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ISprite>> m_sprites;
		fuInt m_Interval = 1;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		size_t GetCount() const noexcept { return m_sprites.size(); }
		LuaSTG::Core::Graphics::ISprite* GetSprite(fuInt index);
		fuInt GetSpriteIndexByTimer(int ani_timer);
		LuaSTG::Core::Graphics::ISprite* GetSpriteByTimer(int ani_timer);
		fuInt GetInterval() const noexcept { return m_Interval; }
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
