#pragma once
#include "GameResource/ResourceBase.hpp"
#include "core/Graphics/Renderer.hpp"

namespace luastg
{
	struct IResourceModel : public IResourceBase
	{
		virtual core::Graphics::IModel* GetModel() = 0;
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/luastg.IResourceModel
	template<> constexpr InterfaceId getInterfaceId<luastg::IResourceModel>() { return UUID::parse("8010a289-8032-5fb3-8867-fb292dbb57df"); }
}
