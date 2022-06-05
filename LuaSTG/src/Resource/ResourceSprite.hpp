#pragma once
#include "ResourceBase.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTGPlus
{
	class ResSprite : public Resource
	{
	private:
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::ISprite> m_sprite;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.0;
		double m_HalfSizeY = 0.0;
		bool m_bRectangle = false;
	public:
		LuaSTG::Core::Graphics::ISprite* GetSprite() noexcept { return m_sprite.get(); }
		BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
		void SetBlendMode(BlendMode m) noexcept { m_BlendMode = m; }
		double GetHalfSizeX() const noexcept { return m_HalfSizeX; }
		double GetHalfSizeY() const noexcept { return m_HalfSizeY; }
		bool IsRectangle() const noexcept { return m_bRectangle; }
	public:
		ResSprite(const char* name, LuaSTG::Core::Graphics::ISprite* sprite, double hx, double hy, bool rect)
			: Resource(ResourceType::Sprite, name)
			, m_sprite(sprite)
			, m_HalfSizeX(hx)
			, m_HalfSizeY(hy)
			, m_bRectangle(rect)
		{
		}
		~ResSprite()
		{
		}
	};
}
