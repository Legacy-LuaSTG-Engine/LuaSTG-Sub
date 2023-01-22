#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Sprite.hpp"

namespace LuaSTGPlus
{
	struct IResourceSprite : public IResourceBase
	{
		virtual Core::Graphics::ISprite* GetSprite() = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual void SetBlendMode(BlendMode m) = 0;
		virtual double GetHalfSizeX() = 0;
		virtual double GetHalfSizeY() = 0;
		virtual bool IsRectangle() = 0;
	};
}
