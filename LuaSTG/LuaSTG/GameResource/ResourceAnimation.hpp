#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTGPlus
{
	struct IResourceAnimation : public IResourceBase
	{
		virtual size_t GetCount() = 0;
		virtual Core::Graphics::ISprite* GetSprite(uint32_t index) = 0;
		virtual uint32_t GetSpriteIndexByTimer(int ani_timer) = 0;
		virtual Core::Graphics::ISprite* GetSpriteByTimer(int ani_timer) = 0;
		virtual uint32_t GetInterval() = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual void SetBlendMode(BlendMode m) = 0;
		virtual double GetHalfSizeX() = 0;
		virtual double GetHalfSizeY() = 0;
		virtual bool IsRectangle() = 0;
	};
}
