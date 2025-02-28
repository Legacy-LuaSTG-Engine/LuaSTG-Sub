#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTGPlus
{
	struct IResourceModel : public IResourceBase
	{
		virtual Core::Graphics::IModel* GetModel() = 0;
	};
}
