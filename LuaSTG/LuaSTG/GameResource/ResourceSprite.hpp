#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTGPlus
{
	class ResSprite : public Resource
	{
	private:
		Core::ScopeObject<Core::Graphics::ISprite> m_sprite;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.0;
		double m_HalfSizeY = 0.0;
		bool m_bRectangle = false;
	public:
		Core::Graphics::ISprite* GetSprite() noexcept { return m_sprite.get(); }
		BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
		void SetBlendMode(BlendMode m) noexcept { m_BlendMode = m; }
		double GetHalfSizeX() const noexcept { return m_HalfSizeX; }
		double GetHalfSizeY() const noexcept { return m_HalfSizeY; }
		bool IsRectangle() const noexcept { return m_bRectangle; }
	public:
		ResSprite(const char* name, Core::Graphics::ISprite* sprite, double hx, double hy, bool rect);
		~ResSprite();
	};
}
