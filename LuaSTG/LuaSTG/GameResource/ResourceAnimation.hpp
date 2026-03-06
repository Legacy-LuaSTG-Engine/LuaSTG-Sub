#pragma once
#include "GameResource/ResourceBase.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "core/Graphics/Sprite.hpp"

namespace luastg
{
	struct IResourceAnimation : public IResourceBase
	{
		virtual size_t GetCount() = 0;
		virtual IResourceSprite* GetSprite(uint32_t index) = 0;
		virtual uint32_t GetSpriteIndexByTimer(int ani_timer) = 0;
		virtual IResourceSprite* GetSpriteByTimer(int ani_timer) = 0;
		virtual uint32_t GetInterval() = 0;
		virtual void SetVertexColor(core::Color4B color) = 0;
		virtual void SetVertexColor(core::Color4B color[4]) = 0;
		virtual void GetVertexColor(core::Color4B color[4]) = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual void SetBlendMode(BlendMode m) = 0;
		virtual double GetHalfSizeX() = 0;
		virtual double GetHalfSizeY() = 0;
		virtual bool IsRectangle() = 0;
		virtual bool IsSpriteCloned() = 0;
		virtual void Render(int timer, float x, float y, float rot, float hscale, float vscale, float z = 0.5f) = 0;
		virtual void Render(int timer, float x, float y, float rot, float hscale, float vscale, BlendMode blend, core::Color4B color, float z = 0.5f) = 0;
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceAnimation
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceAnimation>() { return UUID::parse("8b4a13a3-01e8-5eec-9c5a-0af16bd342d9"); }
}
