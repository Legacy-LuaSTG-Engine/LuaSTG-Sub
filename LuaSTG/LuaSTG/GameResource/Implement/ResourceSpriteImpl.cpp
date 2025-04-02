#include "GameResource/Implement/ResourceSpriteImpl.hpp"
#include "AppFrame.h"

namespace luastg
{
	ResourceSpriteImpl::ResourceSpriteImpl(const char* name, Core::Graphics::ISprite* sprite, double hx, double hy, bool rect)
		: ResourceBaseImpl(ResourceType::Sprite, name)
		, m_sprite(sprite)
		, m_HalfSizeX(hx)
		, m_HalfSizeY(hy)
		, m_bRectangle(rect)
	{
	}

	void ResourceSpriteImpl::RenderRect(float l, float r, float b, float t, float z)
	{
		Core::Graphics::ISprite* pSprite = GetSprite();
		// 备份状态
		float const z_backup = pSprite->getZ();
		// 设置状态
		pSprite->setZ(z);
		// 渲染
		LAPP.updateGraph2DBlendMode(GetBlendMode());
		pSprite->draw(Core::RectF(l, t, r, b));
		// 恢复状态
		pSprite->setZ(z_backup);
	}
	void ResourceSpriteImpl::Render(float x, float y, float rot, float hscale, float vscale, float z)
	{
		Core::Graphics::ISprite* pSprite = GetSprite();
		// 备份状态
		float const z_backup = pSprite->getZ();
		// 设置状态
		pSprite->setZ(z);
		// 渲染
		LAPP.updateGraph2DBlendMode(GetBlendMode());
		pSprite->draw(Core::Vector2F(x, y), Core::Vector2F(hscale, vscale), rot);
		// 恢复状态
		pSprite->setZ(z_backup);
	}
	void ResourceSpriteImpl::Render(float x, float y, float rot, float hscale, float vscale, BlendMode blend, Core::Color4B color, float z)
	{
		Core::Graphics::ISprite* pSprite = GetSprite();
		// 备份状态
		BlendMode blend_backup = GetBlendMode();
		Core::Color4B color_backup[4] {
			Core::Color4B(0xFFFFFFFF),
			Core::Color4B(0xFFFFFFFF),
			Core::Color4B(0xFFFFFFFF),
			Core::Color4B(0xFFFFFFFF),
		};
		pSprite->getColor(color_backup);
		// 设置状态
		SetBlendMode(blend);
		pSprite->setColor(color);
		// 渲染
		Render(x, y, rot, hscale, vscale, z);
		// 恢复状态
		SetBlendMode(blend_backup);
		pSprite->setColor(color_backup);
	}
	void ResourceSpriteImpl::Render4V(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4)
	{
		LAPP.updateGraph2DBlendMode(GetBlendMode());
		GetSprite()->draw(Core::Vector3F(x1, y1, z1),
			Core::Vector3F(x2, y2, z2),
			Core::Vector3F(x3, y3, z3),
			Core::Vector3F(x4, y4, z4));
	}
}
