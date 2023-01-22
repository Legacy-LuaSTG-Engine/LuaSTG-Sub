#include "GameResource/Implement/ResourceSpriteImpl.hpp"

namespace LuaSTGPlus
{
	ResourceSpriteImpl::ResourceSpriteImpl(const char* name, Core::Graphics::ISprite* sprite, double hx, double hy, bool rect)
		: ResourceBaseImpl(ResourceType::Sprite, name)
		, m_sprite(sprite)
		, m_HalfSizeX(hx)
		, m_HalfSizeY(hy)
		, m_bRectangle(rect)
	{
	}
}
