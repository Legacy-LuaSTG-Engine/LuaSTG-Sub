#pragma once
#include "core/SmartReference.hpp"
#include "GameResource/ResourceSprite.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceSpriteImpl : public ResourceBaseImpl<IResourceSprite>
	{
	private:
		core::SmartReference<core::Graphics::ISprite> m_sprite;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.0;
		double m_HalfSizeY = 0.0;
		core::Color4B m_color[4]{
			core::Color4B(255, 255, 255, 255),
			core::Color4B(255, 255, 255, 255),
			core::Color4B(255, 255, 255, 255),
			core::Color4B(255, 255, 255, 255),
		};
		bool m_bRectangle = false;
	public:
		core::Graphics::ISprite* GetSprite() override { return m_sprite.get(); }
		BlendMode GetBlendMode() override { return m_BlendMode; }
		void SetBlendMode(BlendMode m) override { m_BlendMode = m; }
		void SetColor(const core::Color4B color) override {
			m_color[0] = color;
			m_color[1] = color;
			m_color[2] = color;
			m_color[3] = color;
		}
		void SetColor(const core::Color4B c1, const core::Color4B c2, const core::Color4B c3, const core::Color4B c4) override {
			m_color[0] = c1;
			m_color[1] = c2;
			m_color[2] = c3;
			m_color[3] = c4;
		}
		double GetHalfSizeX() override { return m_HalfSizeX; }
		double GetHalfSizeY() override { return m_HalfSizeY; }
		bool IsRectangle() override { return m_bRectangle; }
		void RenderRect(float l, float r, float b, float t, float z) override;
		void Render(float x, float y, float rot, float hscale, float vscale, float z) override;
		void Render(float x, float y, float rot, float hscale, float vscale, BlendMode blend, core::Color4B color, float z) override;
		void Render4V(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) override;
	public:
		ResourceSpriteImpl(const char* name, core::Graphics::ISprite* sprite, double hx, double hy, bool rect);
	};
}
