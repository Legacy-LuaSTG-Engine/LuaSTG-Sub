#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace luastg
{
	struct IResourceModel : public IResourceBase
	{
		virtual Core::Graphics::IModel* GetModel() = 0;
	};
}
