﻿#pragma once
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
		virtual void RenderRect(float l, float r, float b, float t, float z = 0.5f) = 0;
		virtual void Render(float x, float y, float rot, float hscale, float vscale, float z = 0.5f) = 0;
		virtual void Render(float x, float y, float rot, float hscale, float vscale, BlendMode blend, Core::Color4B color, float z = 0.5f) = 0;
		virtual void Render4V(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) = 0;
	};
}
