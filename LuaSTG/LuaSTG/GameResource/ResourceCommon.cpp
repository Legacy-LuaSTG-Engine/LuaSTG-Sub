#include "GameResource/ResourceSprite.hpp"
#include "GameResource/ResourceFX.hpp"
#include "GameResource/ResourceModel.hpp"
#include "AppFrame.h"

namespace LuaSTGPlus
{
	ResSprite::ResSprite(const char* name, Core::Graphics::ISprite* sprite, double hx, double hy, bool rect)
		: Resource(ResourceType::Sprite, name)
		, m_sprite(sprite)
		, m_HalfSizeX(hx)
		, m_HalfSizeY(hy)
		, m_bRectangle(rect)
	{
	}
	ResSprite::~ResSprite()
	{
	}

	ResFX::ResFX(const char* name, const char* path)
		: Resource(ResourceType::FX, name)
	{
		if (!LAPP.GetAppModel()->getRenderer()->createPostEffectShader(path, ~m_shader))
			throw std::runtime_error("ResFX");
	}
	ResFX::~ResFX()
	{
	}
}
